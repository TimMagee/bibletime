/*********
*
* In the name of the Father, and of the Son, and of the Holy Spirit.
*
* This file is part of BibleTime's source code, http://www.bibletime.info/.
*
* Copyright 1999-2015 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License
* version 2.0.
*
**********/

#include "sessionmanager.h"

#include "backend/config/btconfig.h"
#include "backend/managers/cswordbackend.h"
#include "mobile/util/findqmlobject.h"
#include "mobile/ui/btstyle.h"
#include <QGenericReturnArgument>
#include <QMetaObject>
#include <QQuickItem>

namespace btm {

SessionManager::SessionManager(QObject* parent)
    : QObject(parent) {
    m_windowMgr = findQmlObject("WindowManager");
    Q_ASSERT(m_windowMgr != nullptr);
}

void SessionManager::loadDefaultSession() {
    loadWindows();

    int colorTheme = getColorTheme();
    BtStyle::setCurrentStyle(colorTheme);

    int windowArrangementMode = getWindowArrangementMode();
    m_windowMgr->setProperty("windowArrangement", windowArrangementMode);
}

void SessionManager::loadWindows() {
    BtConfig & conf = btConfig();
    Q_FOREACH (const QString & w,
               conf.sessionValue<QStringList>("windowsList"))
    {
        const QString windowGroup = "window/" + w + '/';

        QStringList moduleNames = conf.sessionValue<QStringList>(windowGroup + "modules");
        if (moduleNames.count() == 0)
            continue;
        const QString key = conf.sessionValue<QString>(windowGroup + "key");
        loadWindow(moduleNames, key);
    }
}

void SessionManager::loadWindow(const QStringList& moduleNames, const QString& key) {

    const QString moduleName = moduleNames.at(0); // We don't support parallel yet!
    CSwordModuleInfo * const m = CSwordBackend::instance()->findModuleByName(moduleName);
    if (m == nullptr)
        return;
    CSwordModuleInfo::Category category = m->category();
    const QString categoryName = m->englishCategoryName(category);
    newWindow(categoryName, moduleName, key);
}

void SessionManager::newWindow(const QString& category,
                               const QString& moduleName,
                               const QString& key) {

    QMetaObject::invokeMethod(m_windowMgr, "openWindow",
                              Q_ARG(QVariant, category),
                              Q_ARG(QVariant, moduleName),
                              Q_ARG(QVariant, key));
}

int SessionManager::getWindowArrangementMode() {
    BtConfig & conf = btConfig();
    return conf.sessionValue<int>("MainWindow/MDIArrangementMode");
}

int SessionManager::getColorTheme() {
    BtConfig & conf = btConfig();
    return conf.sessionValue<int>("ColorTheme", BtStyle::darkTheme);
}

void SessionManager::saveDefaultSession() {

    BtConfig & conf = btConfig();

    int currentStyle = BtStyle::getCurrentStyle();
    conf.setSessionValue("ColorTheme",currentStyle);

    int windowArrangementMode = m_windowMgr->property("windowArrangement").toInt();
    conf.setSessionValue("MainWindow/MDIArrangementMode", windowArrangementMode);


    int windowCount = getWindowCount();
    QStringList windowsList;
    for (int windowIndex = 0; windowIndex < windowCount; ++windowIndex)
    {
        windowsList.append(QString::number(windowIndex));
        saveWindowStateToConfig(windowIndex);
    }
    conf.setSessionValue("windowsList", windowsList);
}

int SessionManager::getWindowCount() {
    QVariant windowCountV;
    QMetaObject::invokeMethod(m_windowMgr, "getWindowCount", Q_RETURN_ARG(QVariant, windowCountV));

    int windowCount = 0;
    if (windowCountV.canConvert(QMetaType::Int))
            windowCount = windowCountV.toInt();
    return windowCount;
}

void SessionManager::saveWindowStateToConfig(int windowIndex) {

    QMetaObject::invokeMethod(m_windowMgr, "saveWindowStateToConfig",
                              Q_ARG(QVariant, windowIndex));
}
}
