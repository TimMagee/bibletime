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

#ifndef BTFONTSETTINGS_H
#define BTFONTSETTINGS_H

#include "frontend/bookshelfmanager/btconfigdialog.h"

#include <QMap>
#include <QWidget>
#include "backend/config/btconfig.h"


class BtFontChooserWidget;
class CConfigurationDialog;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;

class BtFontSettingsPage: public BtConfigDialog::Page {

        Q_OBJECT

    private: /* Types: */

        typedef QMap<QString, BtConfig::FontSettingsPair> FontMap;

    public: /* Methods: */

        BtFontSettingsPage(CConfigurationDialog *parent = nullptr);

        void save() const;

    protected slots:

        // This slot is called when the "Use own font for language" button was clicked.
        void useOwnFontClicked(bool);

        // Called when a new font in the fonts page was selected.
        void newDisplayWindowFontSelected(const QFont &);

        // Called when the combobox contents is changed
        void newDisplayWindowFontAreaSelected(const QString&);

    private: /* Methods: */

        void retranslateUi();

    private: /* Fields: */

        QGroupBox *m_fontsGroupBox;
        QLabel *m_languageLabel;
        QComboBox *m_languageComboBox;
        QCheckBox *m_languageCheckBox;
        BtFontChooserWidget* m_fontChooser;

        FontMap m_fontMap;

};

#endif
