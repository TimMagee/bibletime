/*********
*
* This file is part of BibleTime's source code, http://www.bibletime.info/.
*
* Copyright 1999-2015 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License version 2.0.
*
**********/

#include "frontend/settingsdialogs/cswordsettings.h"

#include <QVBoxLayout>
#include "bibletimeapp.h"
#include "frontend/settingsdialogs/btstandardworkstab.h"
#include "frontend/settingsdialogs/bttextfilterstab.h"
#include "frontend/settingsdialogs/cconfigurationdialog.h"
#include "util/cresmgr.h"


CSwordSettingsPage::CSwordSettingsPage(CConfigurationDialog * parent)
        : BtConfigDialog::Page(CResMgr::settings::sword::icon(), parent)
{
    static const QString nullString;

    m_tabWidget = new QTabWidget(this);
        m_worksTab = new BtStandardWorksTab(this);
        m_tabWidget->addTab(m_worksTab, nullString);

        m_filtersTab = new BtTextFiltersTab(this);
        m_tabWidget->addTab(m_filtersTab, nullString);


    QVBoxLayout * mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_tabWidget);

    retranslateUi();
}

void CSwordSettingsPage::retranslateUi() {
    setHeaderText(tr("Desk"));

    m_tabWidget->setTabText(m_tabWidget->indexOf(m_worksTab), tr("Standard works"));
    m_tabWidget->setTabText(m_tabWidget->indexOf(m_filtersTab), tr("Text filters"));
}

void CSwordSettingsPage::save() {
    m_worksTab->save();
    m_filtersTab->save();
}
