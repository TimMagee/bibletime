/*********
*
* In the name of the Father, and of the Son, and of the Holy Spirit.
*
* This file is part of BibleTime's source code, http://www.bibletime.info/.
*
* Copyright 1999-2015 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License version 2.0.
*
**********/

#ifndef CLANGUAGEMGR_H
#define CLANGUAGEMGR_H

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>


/**
  \brief Manages the languages and provides functions to work with them.
  \note This is a singleton.
*/
class CLanguageMgr {

    public:

        /**
          \brief A language descriptor for CLanguageMgr.

          This class (Language) contains the information about the a language.
        */
        class Language {

            friend class CLanguageMgr;
            friend class BtFontSettingsPage;

            public: /* Methods: */

                /**
                  \returns the abbreviation of the this language.
                */
                inline const QString & abbrev() const {
                    if (m_abbrev.isEmpty() && m_altAbbrevs.count()) {
                        /* No standard abbrev but alternative ones */
                        return m_altAbbrevs.first();
                    }
                    return m_abbrev;
                }

                /**
                  \returns the translated name of this language.
                */
                inline const QString & translatedName() const {
                    return m_translatedName;
                }

                /**
                  \returns the english name of this language.
                */
                inline const QString & englishName() const {
                    return m_englishName;
                }

                /**
                  \returns a list of alternative abbreviations for this language.
                */
                inline const QStringList & alternativeAbbrevs() const {
                    return m_altAbbrevs;
                }

                /**
                  \returns whether this language object is valid, i.e. has an
                           abbreviation and an english name.
                */
                inline bool isValid() const {
                    return (!m_abbrev.isEmpty() && !m_englishName.isEmpty());
                }

            private: /* Methods: */

                inline Language() {}

                inline Language(const char * abbrev,
                                const char * englishName,
                                const QString & translatedName)
                    : m_abbrev(abbrev)
                    , m_englishName(QString::fromUtf8(englishName))
                    , m_translatedName(translatedName) {}

                inline Language(const QString & abbrev,
                                const QString & englishName,
                                const QString & translatedName)
                    : m_abbrev(abbrev)
                    , m_englishName(englishName)
                    , m_translatedName(translatedName) {}

                inline Language(const char * abbrev,
                                const char * englishName,
                                const QString & translatedName,
                                const QStringList & altAbbrevs)
                    : m_abbrev(abbrev)
                    , m_englishName(QString::fromUtf8(englishName))
                    , m_translatedName(translatedName)
                    , m_altAbbrevs(altAbbrevs) {}

            private: /* Fields: */

                const QString m_abbrev;
                const QString m_englishName;
                const QString m_translatedName;
                const QStringList m_altAbbrevs;

        }; /* class Language { */

        typedef QList<Language*> LanguageList;
        typedef QHash<QString, const Language*> LangMap;
        typedef QHash<QString, const Language*>::const_iterator LangMapIterator;


        /** Returns the singleton instance, creating it if one does not exist. */
        static CLanguageMgr *instance();

        /** Destroys the singleton instance, if one exists. */
        static void destroyInstance();

        CLanguageMgr();

        virtual ~CLanguageMgr();

        /**
        * Returns the standard languages available as standard. Does nothing for Sword.
        * @return A LangMap map which contains all known languages
        */
        inline const CLanguageMgr::LangMap* languages() const {
            return &m_langMap;
        }
        /**
        * Returns the languages which are available. The languages cover all available modules, but nothing more.
        * @return A map of all languages with modules available for them
        */
        const CLanguageMgr::LangMap& availableLanguages();
        /** Language for abbreviation.
        * @param abbrev The language abbreviation
        * @return Pointer to a language for the given string abbreviation.
        */
        const CLanguageMgr::Language* languageForAbbrev( const QString& abbrev ) const;

        /** Language for translated language name.
        * @param abbrev The translated language name
        * @return Pointer to a language for the given translated language name
        */
        const CLanguageMgr::Language* languageForTranslatedName( const QString& language ) const;
        /** Default language so we don't return NULL pointers.
        * @return Pointer to the default language
        */
        inline const CLanguageMgr::Language* defaultLanguage() const {
            return &m_defaultLanguage;
        }

    private:
        void init();
        inline const QStringList makeStringList(const QString& abbrevs) {
            return abbrevs.split( ";", QString::KeepEmptyParts, Qt::CaseSensitive );
        }

        Language m_defaultLanguage;
        mutable LanguageList m_langList;
        mutable LangMap m_langMap;
        mutable LanguageList m_cleanupLangPtrs;

        struct ModuleCache {
            int moduleCount;
            LangMap availableLanguages;
        } m_availableModulesCache;

        static CLanguageMgr *m_instance;
};

#endif

