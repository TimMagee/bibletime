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

#include "bibletime.h"
#include "backend/config/btconfig.h"
#include "backend/managers/cswordbackend.h"
#include "backend/bookshelfmodel/btbookshelftreemodel.h"
#include "mobile/bibletimeapp.h"
#include "mobile/bookshelfmanager/installmanager.h"
#include "mobile/models/searchmodel.h"
#include "mobile/sessionmanager/sessionmanager.h"
#include "mobile/ui/btstyle.h"
#include "mobile/ui/btsearchinterface.h"
#include "mobile/ui/btwindowinterface.h"
#include "mobile/ui/moduleinterface.h"
#include "mobile/ui/qtquick2applicationviewer.h"
#include "mobile/ui/viewmanager.h"
#include "util/findqmlobject.h"
#include <QBrush>
#include <QColor>
#include <QDateTime>
#include <QGuiApplication>
#include <QPalette>
#include <QQuickItem>
#include <QQmlDebuggingEnabler>
#include <QMetaType>
#include <QTranslator>
#include "util/directory.h"

btm::ViewManager* mgr = nullptr;
btm::SessionManager* sessionMgr = nullptr;
static QFont defaultFont;

void register_gml_classes() {
    QQmlDebuggingEnabler enabler;

    qmlRegisterType<btm::BtWindowInterface>("BibleTime", 1, 0, "BtWindowInterface");
    qmlRegisterType<btm::BtStyle>("BibleTime", 1, 0, "BtStyle");
    qmlRegisterType<btm::InstallManager>("BibleTime", 1, 0, "InstallManager");
    qmlRegisterType<btm::ModuleInterface>("BibleTime", 1, 0, "ModuleInterface");
    qmlRegisterType<btm::SearchModel>("BibleTime", 1, 0, "SearchModel");
    qmlRegisterType<btm::BtSearchInterface>("BibleTime", 1, 0, "BtSearchInterface");
}

btm::ViewManager* getViewManager() {
    return mgr;
}

QFont getDefaultFont() {
    return defaultFont;
}

void openBookshelfManager() {
    QQuickItem* item = btm::findQmlObject("startupBookshelfManager");
    Q_ASSERT(item != nullptr);
    if (item == nullptr)
        return;

    item->setProperty("visible", true);

}

#if defined(Q_OS_WIN) || defined(Q_OS_ANDROID)
// Copies locale.qrc files into home sword directory under locales.d directory
static void installSwordLocales(QDir& homeSword)
{
    QDir sourceSwordLocales(":/sword/locales.d");
    if (!sourceSwordLocales.exists())
        return;

    QStringList filters;
    filters << "*.conf";
    QFileInfoList fileInfoList = sourceSwordLocales.entryInfoList(filters);

    if (!homeSword.exists("locales.d"))
        homeSword.mkdir(("locales.d"));
    homeSword.cd("locales.d");

    for (auto sourceFileInfo : fileInfoList) {

        QString fileName = sourceFileInfo.fileName();
        QString sourceFilePath = sourceFileInfo.absoluteFilePath();
        QFile sourceFile(sourceFilePath);

        QFileInfo destinationFileInfo(homeSword, fileName);
        QString destinationFilePath = destinationFileInfo.absoluteFilePath();
        QFile destinationFile(destinationFileInfo.absoluteFilePath());

        destinationFile.remove();
        sourceFile.copy(destinationFilePath);
    }
}
#endif

/*******************************************************************************
  Handle Qt's meta type system.
*******************************************************************************/

void registerMetaTypes() {
    qRegisterMetaType<FilterOptions>("FilterOptions");
    qRegisterMetaType<DisplayOptions>("DisplayOptions");
    qRegisterMetaTypeStreamOperators<BtBookshelfTreeModel::Grouping>("BtBookshelfTreeModel::Grouping");

    qRegisterMetaType<BTModuleTreeItem::Grouping>("Grouping");
    qRegisterMetaTypeStreamOperators<BTModuleTreeItem::Grouping>("Grouping");

    qRegisterMetaType<BtConfig::StringMap>("StringMap");
    qRegisterMetaTypeStreamOperators<BtConfig::StringMap>("StringMap");

    qRegisterMetaType<QList<int> >("QList<int>");
    qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");
}


int main(int argc, char *argv[]) {
    namespace DU = util::directory;

    BibleTimeApp app(argc, argv); //for QApplication

    registerMetaTypes();

    defaultFont = app.font();

    if (!DU::initDirectoryCache()) {
        qFatal("Error initializing directory cache!");
        return EXIT_FAILURE;
    }

#if defined(Q_OS_WIN) || defined(Q_OS_ANDROID)
    // change directory to the Sword or .sword directory in the $HOME dir so that
    // the sword.conf is found. It points to the sword/locales.d directory
    // This is also needed for the AugmentPath or DataPath to work
    QString homeSwordDir = util::directory::getUserHomeSwordDir().absolutePath();
    QDir dir;
    dir.setCurrent(homeSwordDir);
#endif

    app.startInit();
    if (!app.initBtConfig()) {
        return EXIT_FAILURE;
    }

#if defined(Q_OS_WIN) || defined(Q_OS_ANDROID)
    if (btm::BtStyle::getAppVersion() > btConfig().value<QString>("btm/version")) {
        installSwordLocales(dir);
        btConfig().setValue<QString>("btm/version", btm::BtStyle::getAppVersion());
    }
#endif

    //first install QT's own translations
    QTranslator qtTranslator;
    QString locale = QLocale::system().name();
    qtTranslator.load("qt_" + locale);
    app.installTranslator(&qtTranslator);
    //then our own
    QTranslator bibleTimeTranslator;
    bibleTimeTranslator.load( QString("mobile_ui_").append(locale), DU::getLocaleDir().canonicalPath());
    app.installTranslator(&bibleTimeTranslator);

    // Initialize display template manager:
    if (!app.initDisplayTemplateManager()) {
        qFatal("Error initializing display template manager!");
       return EXIT_FAILURE;
    }

    register_gml_classes();

    mgr = new btm::ViewManager;
    mgr->show();

    btm::BibleTime btm;

    sessionMgr = new btm::SessionManager();
    int installedModules = CSwordBackend::instance()->moduleList().count();
    if (installedModules == 0) {
        openBookshelfManager();
    }
    else {
        sessionMgr->loadDefaultSession();
    }

    int rtn = app.exec();
    sessionMgr->saveDefaultSession();
    return rtn;
}
