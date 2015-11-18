/*********
*
* This file is part of BibleTime's source code, http://www.bibletime.info/.
*
* Copyright 1999-2015 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License version 2.0.
*
**********/

#include "frontend/bookshelfmanager/installpage/btinstallpageworkswidget.h"

#include <QApplication>
#include <QAction>
#include <QDebug>
#include <QTimer>
#include <QToolButton>
#include "backend/btinstallbackend.h"
#include "backend/managers/cswordbackend.h"
#include "bibletimeapp.h"
#include "frontend/bookshelfmanager/installpage/btinstallpage.h"
#include "frontend/bookshelfmanager/installpage/btinstallpagemodel.h"
#include "frontend/bookshelfmanager/installpage/btrefreshprogressdialog.h"
#include "frontend/btbookshelfview.h"
#include "util/cresmgr.h"


namespace {

/** Filters out already installed modules which can't be updated right now. */
bool filter(const CSwordModuleInfo *mInfo) {
    typedef CSwordModuleInfo CSMI;
    typedef sword::SWVersion SV;

    const CSMI *installedModule = CSwordBackend::instance()->findModuleByName(mInfo->name());
    if (installedModule) {
        // Already installed, check if it's an update:
        const SV curVersion(installedModule->config(CSMI::ModuleVersion).toLatin1());
        const SV newVersion(mInfo->config(CSMI::ModuleVersion).toLatin1());
        if (curVersion >= newVersion) {
            return false;
        }
    }
    return true;
}

}

BtInstallPageWorksWidget::BtInstallPageWorksWidget(
        const sword::InstallSource &source,
        const BtBookshelfTreeModel::Grouping &g,
        BtInstallPage *parent, Qt::WindowFlags flags)
            : BtBookshelfWidget(parent, flags)
            , m_source(source)
            , m_parent(parent)
            , m_backend(nullptr)
            , m_myModel(nullptr)
{

    setTreeModel(new BtInstallPageModel(g, this));

    treeView()->setHeaderHidden(false);
    showHideButton()->hide();
    showHideAction()->setVisible(false);

    m_sourceRefreshButton = new QToolButton(this);
    m_sourceRefreshButton->setAutoRaise(true);
    m_sourceRefreshButton->setToolTip(tr("Refresh the list of works from this source"));
    m_sourceRefreshButton->setIcon(CResMgr::bookshelfmgr::installpage::icon_refresh());
    setRightCornerWidget(m_sourceRefreshButton);

    connect(m_sourceRefreshButton, SIGNAL(clicked()),
            this,                  SLOT(slotSourceRefresh()));

    m_backend = BtInstallBackend::backend(m_source);
    Q_ASSERT(m_backend != nullptr);
    m_myModel = new BtBookshelfModel(this);
    Q_FOREACH(CSwordModuleInfo * const module, m_backend->moduleList())
        if (filter(module))
            m_myModel->addModule(module);
    setSourceModel(m_myModel);
}

BtInstallPageWorksWidget::~BtInstallPageWorksWidget() {
    delete m_backend;
}

void BtInstallPageWorksWidget::deleteSource() {
    qDebug() << "Deleting source" << m_source.caption;

    m_myModel->clear();
    BtInstallBackend::deleteSource(QString(m_source.caption));
}

void BtInstallPageWorksWidget::updateTree() {
    qDebug() << "Updating BtInstallPageWorksWidget tree for source" << m_source.caption;

    m_myModel->clear();

    // Is this necessary?
    m_source = BtInstallBackend::source(QString(m_source.caption));
    m_backend = BtInstallBackend::backend(m_source);

    // Repopulate model:
    Q_FOREACH(CSwordModuleInfo * const module, m_backend->moduleList())
        if (filter(module))
            m_myModel->addModule(module);
}

void BtInstallPageWorksWidget::slotSourceRefresh() {
    qDebug() << "Refreshing source" << m_source.caption;

    if (BtInstallBackend::isRemote(m_source)) {
        BtRefreshProgressDialog d(m_source, this);
        if (!d.runAndDelete()) return;
    }
    updateTree();
}
