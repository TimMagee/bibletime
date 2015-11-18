/*********
*
* This file is part of BibleTime's source code, http://www.bibletime.info/.
*
* Copyright 1999-2015 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License version 2.0.
*
**********/

#include "btconfig.h"

#include <QDebug>
#include <QLocale>
#include "../../util/directory.h" // DU::getUserBaseDir()
#include "../btmoduletreeitem.h"
#include "../managers/cdisplaytemplatemgr.h"
#include "../managers/cswordbackend.h"

// Sword includes:
#include <versekey.h> // For search scope configuration


#define BTCONFIG_API_VERSION 1
namespace {
const QString BTCONFIG_API_VERSION_KEY = "btconfig_api_version";
}


/*
 * set the instance variable initially to 0, so it can be safely checked
 * whether the variable has been initialized yet.
 */
BtConfig * BtConfig::m_instance = nullptr;

BtConfig::StringMap BtConfig::m_defaultSearchScopes;


BtConfig::BtConfig(const QString & settingsFile)
    : BtConfigCore(settingsFile)
{
    Q_ASSERT_X(!m_instance, "BtConfig", "Already initialized!");
    m_instance = this;

    if (m_defaultSearchScopes.isEmpty()) {
        m_defaultSearchScopes.insert(tr("Old testament"),          QString("Gen - Mal"));
        m_defaultSearchScopes.insert(tr("Moses/Pentateuch/Torah"), QString("Gen - Deut"));
        m_defaultSearchScopes.insert(tr("History"),                QString("Jos - Est"));
        m_defaultSearchScopes.insert(tr("Prophets"),               QString("Isa - Mal"));
        m_defaultSearchScopes.insert(tr("New testament"),          QString("Mat - Rev"));
        m_defaultSearchScopes.insert(tr("Gospels"),                QString("Mat - Joh"));
        m_defaultSearchScopes.insert(tr("Letters/Epistles"),       QString("Rom - Jude"));
        m_defaultSearchScopes.insert(tr("Paul's Epistles"),        QString("Rom - Phile"));
    }
}

BtConfig::InitState BtConfig::initBtConfig() {
    Q_ASSERT(!m_instance);

    const QString confFileName = util::directory::getUserBaseDir().absolutePath()
                                 + "/bibletimerc";
    bool confExisted = QFile::exists(confFileName);
    m_instance = new BtConfig(confFileName);
    if (!confExisted) {
        m_instance->setValue<int>(BTCONFIG_API_VERSION_KEY, BTCONFIG_API_VERSION);
        return INIT_OK;
    }

    int btConfigOldApi = m_instance->value<int>(BTCONFIG_API_VERSION_KEY, 0);
    if (btConfigOldApi == BTCONFIG_API_VERSION)
        return INIT_OK;
    return (btConfigOldApi < BTCONFIG_API_VERSION)
           ? INIT_NEED_UNIMPLEMENTED_FORWARD_MIGRATE
           : INIT_NEED_UNIMPLEMENTED_BACKWARD_MIGRATE;
}

void BtConfig::forceMigrate()
{ m_instance->setValue<int>(BTCONFIG_API_VERSION_KEY, BTCONFIG_API_VERSION); }

BtConfig& BtConfig::getInstance() {
    Q_ASSERT_X(m_instance, "BtConfig", "Not yet initialized!");
    return *m_instance;
}

void BtConfig::destroyInstance() {
    delete m_instance;
    m_instance = nullptr;
}

void BtConfig::setModuleEncryptionKey(const QString & name,
                                      const QString & key)
{
    Q_ASSERT(!name.isEmpty());
    setValue("Module keys/" + name, key);
}

QString BtConfig::getModuleEncryptionKey(const QString & name) {
    Q_ASSERT(!name.isEmpty());
    return value<QString>("Module keys/" + name, QString::null);
}

QHash<QString, QList<QKeySequence> > BtConfig::getShortcuts(const QString & shortcutGroup) {
    beginGroup(shortcutGroup);
        QHash<QString, QList<QKeySequence> > allShortcuts;
        Q_FOREACH (const QString & key, childKeys()) {
            QVariant variant = qVariantValue(key);

            QList<QKeySequence> shortcuts;

            if (variant.type() == QVariant::List) { // For BibleTime before 2.9
                Q_FOREACH (const QVariant & shortcut, variant.toList())
                    shortcuts.append(shortcut.toString());
            } else if (variant.type() == QVariant::StringList
                       || variant.type() == QVariant::String)
            { // a StringList with one element is recognized as a QVariant::String
                Q_FOREACH (const QString & shortcut, variant.toStringList())
                    shortcuts.append(shortcut);
            } else { // it's something we don't know, skip it
                continue;
            }

            allShortcuts.insert(key, shortcuts);
        }
    endGroup();
    return allShortcuts;
}

void BtConfig::setShortcuts(const QString & shortcutGroup,
                            const QHash<QString, QList<QKeySequence> > & shortcuts)
{
    typedef QHash<QString, QList<QKeySequence> >::const_iterator SHMCI;

    beginGroup(shortcutGroup);
        for (SHMCI it = shortcuts.begin(); it != shortcuts.end(); ++it) {
            // Write beautiful string lists (since 2.9):
            /// \note saving QKeySequences directly doesn't appear to work!
            QStringList varList;
            Q_FOREACH (const QKeySequence & shortcut, it.value())
                varList.append(shortcut.toString());

            if (!varList.empty())
                setValue(it.key(), varList);
        }
    endGroup();
}

FilterOptions BtConfig::getFilterOptions() {
    FilterOptions os;
    beginGroup("presentation");
    os.footnotes           = sessionValue<bool>("footnotes", true);
    os.strongNumbers       = sessionValue<bool>("strongNumbers", true);
    os.headings            = sessionValue<bool>("headings", true);
    os.morphTags           = sessionValue<bool>("morphTags", true);
    os.lemmas              = sessionValue<bool>("lemmas", true);
    os.redLetterWords      = sessionValue<bool>("redLetterWords", true);
    os.hebrewPoints        = sessionValue<bool>("hebrewPoints", true);
    os.hebrewCantillation  = sessionValue<bool>("hebrewCantillation", true);
    os.greekAccents        = sessionValue<bool>("greekAccents", true);
    os.textualVariants     = sessionValue<bool>("textualVariants", false);
    os.scriptureReferences = sessionValue<bool>("scriptureReferences", true);
    os.morphSegmentation   = sessionValue<bool>("morphSegmentation", true);
    endGroup();
    return os;
}

void BtConfig::setFilterOptions(const FilterOptions & os) {
    beginGroup("presentation");
    setSessionValue("footnotes", static_cast<bool>(os.footnotes));
    setSessionValue("strongNumbers", static_cast<bool>(os.strongNumbers));
    setSessionValue("headings", static_cast<bool>(os.headings));
    setSessionValue("morphTags", static_cast<bool>(os.morphTags));
    setSessionValue("lemmas", static_cast<bool>(os.lemmas));
    setSessionValue("redLetterWords", static_cast<bool>(os.redLetterWords));
    setSessionValue("hebrewPoints", static_cast<bool>(os.hebrewPoints));
    setSessionValue("hebrewCantillation", static_cast<bool>(os.hebrewCantillation));
    setSessionValue("greekAccents", static_cast<bool>(os.greekAccents));
    setSessionValue("textualVariants", static_cast<bool>(os.textualVariants));
    setSessionValue("scriptureReferences", static_cast<bool>(os.scriptureReferences));
    setSessionValue("morphSegmentation", static_cast<bool>(os.morphSegmentation));
    endGroup();
}

DisplayOptions BtConfig::getDisplayOptions() {
    DisplayOptions os;
    beginGroup("presentation");
    os.lineBreaks   = sessionValue<bool>("lineBreaks", false);
    os.verseNumbers = sessionValue<bool>("verseNumbers", false);
    endGroup();
    return os;
}

void BtConfig::setDisplayOptions(const DisplayOptions & os) {
    beginGroup("presentation");
    setSessionValue("lineBreaks", static_cast<bool>(os.lineBreaks));
    setSessionValue("verseNumbers", static_cast<bool>(os.verseNumbers));
    endGroup();
}

void BtConfig::setFontForLanguage(const CLanguageMgr::Language & language,
                                  const FontSettingsPair & fontSettings)
{
    const QString & englishName = language.englishName();
    Q_ASSERT(!englishName.isEmpty());

    QMutexLocker lock(&this->m_mutex);
    // write the language to the settings
    setValue("fonts/" + englishName, fontSettings.second.toString());
    setValue("font standard settings/" + englishName, fontSettings.first);

    // Remove language from the cache:
    m_fontCache.remove(&language);
}

BtConfig::FontSettingsPair BtConfig::getFontForLanguage(
        const CLanguageMgr::Language & language)
{
    const QString & englishName = language.englishName();
    Q_ASSERT(!englishName.isEmpty());

    QMutexLocker lock(&this->m_mutex);
    // Check the cache first:
    FontCacheMap::const_iterator it(m_fontCache.find(&language));
    if (it != m_fontCache.end())
        return *it;

    // Retrieve the font from the settings
    FontSettingsPair fontSettings;

    fontSettings.first = value<bool>("font standard settings/" + englishName, false);

    QFont font;
    if (fontSettings.first) {
        if (!font.fromString(value<QString>("fonts/" + englishName, getDefaultFont().toString()))) {
            /// \todo
        }
    } else {
        font = getDefaultFont();
    }
    fontSettings.second = font;

    // Cache the value:
    m_fontCache.insert(&language, fontSettings);

    return fontSettings;
}

BtConfig::StringMap BtConfig::getSearchScopesForCurrentLocale() {
    StringMap map = value<BtConfig::StringMap>("properties/searchScopes", m_defaultSearchScopes);

    // Convert map to current locale:
    sword::VerseKey vk;
    for (StringMap::Iterator it = map.begin(); it != map.end(); it++) {
        QString &s = it.value();
        sword::ListKey list(vk.parseVerseList(QByteArray(s.toUtf8()), "Genesis 1:1", true));
        s.clear();
        for (int i = 0; i < list.getCount(); i++) {
            s.append(QString::fromUtf8(list.getElement(i)->getRangeText()));
            s.append("; ");
        }
    }
    return map;
}

void BtConfig::setSearchScopesWithCurrentLocale(StringMap searchScopes) {
    /**
     * We want to make sure that the search scopes are saved with english
     * key names so loading them will always work with each locale set.
     */
    sword::VerseKey vk;
    BtConfig::StringMap::Iterator iter = searchScopes.begin();
    while (iter != searchScopes.end()) {
        QString &data = iter.value();
        bool parsingWorked = true;
        sword::ListKey list(vk.parseVerseList(data.toUtf8(), "Genesis 1:1", true));
        data.clear();
        for (int i = 0; i < list.getCount(); i++) {
            sword::VerseKey * verse(dynamic_cast<sword::VerseKey *>(list.getElement(i)));

            if (verse != nullptr) {
                verse->setLocale("en");
                data.append(QString::fromUtf8(verse->getRangeText()));
                data.append(";");
            } else {
                parsingWorked = false;
                break;
            }
        }

        if (parsingWorked)
            iter++;
        else
            iter = searchScopes.erase(iter);
    }
    setValue("properties/searchScopes", searchScopes);
}

void BtConfig::deleteSearchScopesWithCurrentLocale() {
    remove("properties/searchScopes");
}

CSwordModuleInfo *BtConfig::getDefaultSwordModuleByType(const QString & moduleType) {
    const QString moduleName = value<QString>("settings/defaults/" + moduleType, QString());
    if (moduleName.isEmpty())
        return nullptr;

    return CSwordBackend::instance()->findModuleByName(moduleName);
}

void BtConfig::setDefaultSwordModuleByType(const QString &moduleType,
                                           const CSwordModuleInfo * const module)
{
    setValue("settings/defaults/" + moduleType,
             module != nullptr ? module->name() : QString::null);
}

/**
  \todo -CDisplayWindow gets a construct method that reads from config and constructs and
        returns the respective child window (check whether module is installed...)
        -CDisplayWindows get a new variable "id" or something, which is a unique identifier.
        The path in the configuration will use this id as name. (who gives out the IDs?)
        -values are updated as they are changed, just like the rest of bibletime
        -QMdiArea::subWindowActivated signal will trigger reading the window order and saving
        it to the config.
        Action Plan:
        1. get current code to work with old session system
        2. move complete code over to BtConfig
        3. remove CBTConfig
        4. implement BtConfig infrastructure for saving window configuration
         - function to add a window
         - function to remove a window
         - specify how to save ordering
        5. change CDisplayWindows to write all state changes to the configuration
        6. implement BtConfig::readSession and callers
        7. make session handling code work with QSetting paths instead of properties
        8. add gui for new session handling
        9. remove old gui for session handling
*/
