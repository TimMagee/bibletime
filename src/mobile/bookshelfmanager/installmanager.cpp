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

#include "installmanager.h"

#include "backend/btinstallbackend.h"
#include "backend/managers/clanguagemgr.h"
#include "backend/btinstallmgr.h"
#include "mobile/btmmain.h"
#include "mobile/ui/qtquick2applicationviewer.h"
#include "mobile/ui/viewmanager.h"
#include <QDebug>
#include <QQuickItem>
#include <QtAlgorithms>

namespace btm {

enum TextRoles {
    TextRole = Qt::UserRole + 1
};

enum WorksRoles {
    TitleRole = Qt::UserRole + 1,
    DescriptionRole = Qt::UserRole + 2,
    SelectedRole = Qt::UserRole + 3,
    InstalledRole = Qt::UserRole + 4
};

static bool moduleInstalled(const CSwordModuleInfo& moduleInfo) {
    const CSwordModuleInfo *installedModule = CSwordBackend::instance()->findModuleByName(moduleInfo.name());
    return installedModule != nullptr;
}

static void setupTextModel(const QStringList& modelList, RoleItemModel* model) {
    QHash<int, QByteArray> roleNames;
    roleNames[TextRole] =  "modelText";
    model->setRoleNames(roleNames);

    model->clear();
    for (int i=0; i< modelList.count(); ++i) {
        QString source = modelList.at(i);
        QStandardItem* item = new QStandardItem();
        item->setData(source, TextRole);
        model->appendRow(item);
    }
}

static void setupWorksModel(const QStringList& titleList,
                            const QStringList& descriptionList,
                            const QList<int>& installedList,
                            RoleItemModel* model) {
    Q_ASSERT(titleList.count() == descriptionList.count());
    Q_ASSERT(titleList.count() == installedList.count());

    QHash<int, QByteArray> roleNames;
    roleNames[TitleRole] =  "title";
    roleNames[DescriptionRole] = "desc";
    roleNames[SelectedRole] = "selected";
    roleNames[InstalledRole] = "installed";
    model->setRoleNames(roleNames);

    model->clear();
    for (int i=0; i< titleList.count(); ++i) {
        QStandardItem* item = new QStandardItem();
        QString title = titleList.at(i);
        item->setData(title, TitleRole);
        QString description = descriptionList.at(i);
        item->setData(description, DescriptionRole);
        int installed = installedList.at(i);
        item->setData(installed, InstalledRole);
        item->setData(0, SelectedRole);
        model->appendRow(item);
    }
}

InstallManager::InstallManager(QObject* /* parent */)
    : m_installManagerChooserObject(nullptr),
      m_btInstallMgr(nullptr),
      m_backend(nullptr) {

    bool ok = connect(&m_installSourcesManager, SIGNAL(sourcesUpdated()),
                      this, SLOT(updateModels()));
    Q_ASSERT(ok);
}

void InstallManager::openChooser() {

    if (m_installManagerChooserObject == nullptr)
        findInstallManagerObject();
    if (m_installManagerChooserObject == nullptr)
        return;

    m_modulesToInstallRemove.clear();
    setupSourceModel();
    makeConnections();
    setProperties();
    sourceIndexChanged(0);
}

void InstallManager::updateModels() {
    setupSourceModel();
    sourceIndexChanged(0);
}

void InstallManager::findInstallManagerObject() {

    QtQuick2ApplicationViewer* viewer = getViewManager()->getViewer();
    QQuickItem * rootObject = nullptr;
    if (viewer != nullptr)
        rootObject = viewer->rootObject();
    if (rootObject != nullptr)
        m_installManagerChooserObject = rootObject->findChild<QQuickItem*>("installManagerChooser");
}

void InstallManager::findProgressObject() {
    QtQuick2ApplicationViewer* viewer = getViewManager()->getViewer();
    QQuickItem * rootObject = nullptr;
    if (viewer != nullptr)
        rootObject = viewer->rootObject();
    if (rootObject != nullptr)
        m_progressObject = rootObject->findChild<QQuickItem*>("progress");
}


void InstallManager::setupSourceModel() {
    m_sourceList = BtInstallBackend::sourceNameList();
    setupTextModel(m_sourceList, &m_sourceModel);
}

void InstallManager::makeConnections()
{
    m_installManagerChooserObject->disconnect();

    bool ok = connect(m_installManagerChooserObject, SIGNAL(sourceChanged(int)),
                      this, SLOT(sourceIndexChanged(int)));
    Q_ASSERT(ok);

    ok = connect(m_installManagerChooserObject, SIGNAL(categoryChanged(int)),
                      this, SLOT(categoryIndexChanged(int)));
    Q_ASSERT(ok);

    ok = connect(m_installManagerChooserObject, SIGNAL(languageChanged(int)),
                      this, SLOT(languageIndexChanged(int)));
    Q_ASSERT(ok);

    ok = connect(m_installManagerChooserObject, SIGNAL(workSelected(int)),
                      this, SLOT(workSelected(int)));
    Q_ASSERT(ok);

    ok = connect(m_installManagerChooserObject, SIGNAL(cancel()),
                      this, SLOT(cancel()));
    Q_ASSERT(ok);

    ok = connect(m_installManagerChooserObject, SIGNAL(installRemove()),
                      this, SLOT(installRemove()));
    Q_ASSERT(ok);

    ok = connect(m_installManagerChooserObject, SIGNAL(refreshLists()),
                      this, SLOT(refreshLists()));
    Q_ASSERT(ok);
}

void InstallManager::setProperties() {
    m_installManagerChooserObject->setProperty("sourceModel", QVariant::fromValue(&m_sourceModel));
    m_installManagerChooserObject->setProperty("categoryModel", QVariant::fromValue(&m_categoryModel));
    m_installManagerChooserObject->setProperty("languageModel", QVariant::fromValue(&m_languageModel));
    m_installManagerChooserObject->setProperty("worksModel", QVariant::fromValue(&m_worksModel));
    m_installManagerChooserObject->setProperty("sourceIndex", 0);
    m_installManagerChooserObject->setProperty("visible", true);
}

void InstallManager::updateSwordBackend() {
    QString sourceName = getCurrentListItem("sourceIndex", m_sourceList);
    sword::InstallSource source = BtInstallBackend::source(sourceName);
    m_backend = BtInstallBackend::backend(source);
}

void InstallManager::sourceIndexChanged(int index)
{
    if (index < 0 || index >= m_sourceList.count())
        return;

    updateSwordBackend();
    updateCategoryAndLanguageModels();
    updateWorksModel();
}

void InstallManager::categoryIndexChanged(int index)
{
    if (index < 0 || index >= m_categoryList.count())
        return;
    updateWorksModel();
}

void InstallManager::languageIndexChanged(int index)
{
    if (index < 0 || index >= m_languageList.count())
        return;
    updateWorksModel();
}

void InstallManager::workSelected(int index) {
    QStandardItem* item = m_worksModel.item(index,0);
    QVariant vSelected = item->data(SelectedRole);
    int selected = vSelected.toInt();
    selected = selected == 0 ? 1 : 0;
    item->setData(selected, SelectedRole);

    CSwordModuleInfo* moduleInfo = m_worksList.at(index);
    m_modulesToInstallRemove[moduleInfo] = selected == 1;
}

void InstallManager::cancel() {
    m_installManagerChooserObject->setProperty("visible", false);
}

void InstallManager::installRemove() {
    m_installManagerChooserObject->setProperty("visible", false);

    m_modulesToRemove.clear();
    m_modulesToInstall.clear();
    QMap<CSwordModuleInfo*, bool>::const_iterator it;
    for(it=m_modulesToInstallRemove.constBegin();
        it!=m_modulesToInstallRemove.constEnd();
        ++it) {
        CSwordModuleInfo* moduleInfo = it.key();
        bool selected = it.value();
        if ( ! selected)
            continue;
        if (moduleInstalled(*moduleInfo)) {
            m_modulesToRemove.append(moduleInfo);
        }
        else if ( ! moduleInstalled(*moduleInfo)) {
            m_modulesToInstall.append(moduleInfo);
        }
    }
    installModules(m_modulesToInstall);
    removeModules(m_modulesToRemove);
}

void InstallManager::updateCategoryAndLanguageModels()
{
    if (m_backend == nullptr)
        return;
    const QList<CSwordModuleInfo*> modules = m_backend->moduleList();

    QSet<QString> categories;
    QSet<QString> languages;
    for (int moduleIndex=0; moduleIndex<modules.count(); ++moduleIndex) {
        CSwordModuleInfo* module = modules.at(moduleIndex);
        CSwordModuleInfo::Category category = module->category();
        //        QString name = module->name();
        QString categoryName = module->categoryName(category);
        const CLanguageMgr::Language* language = module->language();
        QString languageName = language->englishName();
        categories.insert(categoryName);
        languages.insert(languageName);
    }

    QString currentCategory = getCurrentListItem("categoryIndex", m_categoryList);
        m_categoryList = categories.toList();
    m_categoryList.sort();
    setupTextModel(m_categoryList, &m_categoryModel);
    setCurrentListItem("categoryIndex", m_categoryList, currentCategory);

    QString currentLanguage = getCurrentListItem("languageIndex", m_languageList);
    m_languageList = languages.toList();
    m_languageList.sort();
    setupTextModel(m_languageList, &m_languageModel);
    setCurrentListItem("languageIndex", m_languageList, currentLanguage);
}

QString InstallManager::getCurrentListItem(const char* propertyName, const QStringList& list) {
    QString value;
    QVariant vIndex = m_installManagerChooserObject->property(propertyName);
    bool ok;
    int index = vIndex.toInt(&ok);
    if (ok) {
        if (index >= 0 && index < list.count())
            value = list.at(index);
    }
    return value;
}

void InstallManager::setCurrentListItem(const char* propertyName,
                                              const QStringList& list,
                                              const QString& itemName) {
    int  index = list.indexOf(itemName);
    if (index < 0)
        index = 0;
    m_installManagerChooserObject->setProperty(propertyName, index);
}

void InstallManager::updateWorksModel()
{
    QString sourceName = getCurrentListItem("sourceIndex", m_sourceList);
    QString categoryName = getCurrentListItem("categoryIndex", m_categoryList);
    QString languageName = getCurrentListItem("languageIndex", m_languageList);

    if (m_backend == nullptr)
        return;
    const QList<CSwordModuleInfo*> modules = m_backend->moduleList();

    m_worksTitleList.clear();
    m_worksDescList.clear();
    m_worksList.clear();
    m_worksInstalledList.clear();

    for (int moduleIndex=0; moduleIndex<modules.count(); ++moduleIndex) {
        CSwordModuleInfo* module = modules.at(moduleIndex);
        module->setProperty("installSourceName", sourceName);
        CSwordModuleInfo::Category category = module->category();
        QString moduleCategoryName = module->categoryName(category);
        const CLanguageMgr::Language* language = module->language();
        QString moduleLanguageName = language->englishName();
        if (moduleCategoryName == categoryName &&
            moduleLanguageName == languageName ) {
            QString name = module->name();
            QString description = module->config(CSwordModuleInfo::Description);
            QString version = module->config(CSwordModuleInfo::ModuleVersion);
            QString info = description + ": " + version;\
            int installed = moduleInstalled(*module) ? 1 : 0;
            m_worksTitleList.append(name);
            m_worksDescList.append(info);
            m_worksList.append(module);
            m_worksInstalledList.append(installed);
        }
    }
    setupWorksModel(m_worksTitleList, m_worksDescList, m_worksInstalledList, &m_worksModel);
}

void InstallManager::removeModules(const QList<CSwordModuleInfo*>& modules) {

    QStringList moduleNames;
    Q_FOREACH(CSwordModuleInfo const * const mInfo, modules) {
        QString moduleName = mInfo->name();
        moduleNames.append(moduleName);
    }
    // Update the module list before really removing. Remember deleting the pointers later.
    QList<CSwordModuleInfo*> toBeDeleted = CSwordBackend::instance()->takeModulesFromList(moduleNames);
    Q_ASSERT(toBeDeleted.size() == moduleNames.size());

    sword::InstallMgr installMgr;
    QMap<QString, sword::SWMgr*> mgrDict; //maps config paths to SWMgr objects
    Q_FOREACH(CSwordModuleInfo * const mInfo, toBeDeleted) {
        Q_ASSERT(mInfo); // Only installed modules could have been selected and returned by takeModulesFromList
        // Find the install path for the sword manager
        QString prefixPath = mInfo->config(CSwordModuleInfo::AbsoluteDataPath) + "/";
        QString dataPath = mInfo->config(CSwordModuleInfo::DataPath);
        if (dataPath.left(2) == "./") {
            dataPath = dataPath.mid(2);
        }
        if (prefixPath.contains(dataPath)) { //remove module part to get the prefix path
            prefixPath = prefixPath.remove( prefixPath.indexOf(dataPath), dataPath.length() );
        }
        else { //This is an error, should not happen
            qWarning() << "Removing" << mInfo->name() << "didn't succeed because the absolute path" << prefixPath << "didn't contain the data path" << dataPath;
            continue; // don't remove this, go to next of the for loop
        }

        // Create the sword manager and remove the module
        sword::SWMgr* mgr = mgrDict[ prefixPath ];
        if (!mgr) { //create new mgr if it's not yet available
            mgrDict.insert(prefixPath, new sword::SWMgr(prefixPath.toLocal8Bit()));
            mgr = mgrDict[ prefixPath ];
        }
        qDebug() << "Removing the module" << mInfo->name() << "...";
        installMgr.removeModule(mgr, mInfo->module()->getName());
    }
    //delete the removed moduleinfo pointers
    qDeleteAll(toBeDeleted);
    //delete all mgrs which were created above
    qDeleteAll(mgrDict);
    mgrDict.clear();
}

void InstallManager::installModules(const QList<CSwordModuleInfo*>& modules) {
    m_installProgress.openProgress(modules);
}

void InstallManager::refreshLists() {
    m_installSourcesManager.refreshSources();
}

int InstallManager::getInstalledModuleCount() const {
    return 0;
}
} // end namespace
