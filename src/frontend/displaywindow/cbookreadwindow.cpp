/*********
*
* This file is part of BibleTime's source code, http://www.bibletime.info/.
*
* Copyright 1999-2015 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License version 2.0.
*
**********/

#include "frontend/displaywindow/cbookreadwindow.h"

#include <QAction>
#include <QMenu>
#include <QSplitter>
#include <QToolBar>
#include "bibletime.h"
#include "backend/keys/cswordtreekey.h"
#include "frontend/display/bthtmlreaddisplay.h"
#include "frontend/displaywindow/bttoolbarpopupaction.h"
#include "frontend/displaywindow/btactioncollection.h"
#include "frontend/displaywindow/btmodulechooserbar.h"
#include "frontend/displaywindow/btdisplaysettingsbutton.h"
#include "frontend/keychooser/cbooktreechooser.h"
#include "util/cresmgr.h"
#include "util/tool.h"


void CBookReadWindow::applyProfileSettings(const QString & windowGroup) {
    CLexiconReadWindow::applyProfileSettings(windowGroup);

    Q_ASSERT(m_treeAction);
    Q_ASSERT(windowGroup.endsWith('/'));
    if (btConfig().sessionValue<bool>(windowGroup + "treeShown", true) != m_treeAction->isChecked())
        m_treeAction->activate(QAction::Trigger);
}

void CBookReadWindow::storeProfileSettings(QString const & windowGroup) const {
    CLexiconReadWindow::storeProfileSettings(windowGroup);

    Q_ASSERT(windowGroup.endsWith('/'));
    btConfig().setSessionValue(windowGroup + "treeShown", m_treeAction->isChecked());
}

void CBookReadWindow::initActions() {
    CLexiconReadWindow::initActions();
    BtActionCollection* ac = actionCollection();
    insertKeyboardActions(ac);

    //cleanup, not a clean oo-solution
    QAction *a = ac->action("nextEntry");
    Q_ASSERT(a != nullptr);
    a->setEnabled(false);
    a = ac->action("previousEntry");
    Q_ASSERT(a != nullptr);
    a->setEnabled(false);

    m_treeAction = ac->action("toggleTree");
    Q_ASSERT(m_treeAction != nullptr);
    QObject::connect(m_treeAction, SIGNAL(triggered()),
                     this,         SLOT(treeToggled()));
    addAction(m_treeAction);

    ac->readShortcuts("Book shortcuts");
}

void CBookReadWindow::insertKeyboardActions( BtActionCollection* const a ) {
    QAction* qaction;

    qaction = new QAction( /* QIcon(CResMgr::displaywindows::bookWindow::toggleTree::icon), */
        tr("Toggle tree view"), a);
    qaction->setCheckable(true);
    // qaction->setShortcut(CResMgr::displaywindows::bookWindow::toggleTree::accel);
    a->addAction("toggleTree", qaction);
}

/** No descriptions */
void CBookReadWindow::initConnections() {
    CLexiconReadWindow::initConnections();

    connect(m_treeChooser, SIGNAL(keyChanged(CSwordKey*)), this, SLOT(lookupSwordKey(CSwordKey*)));
    connect(m_treeChooser, SIGNAL(keyChanged(CSwordKey*)), keyChooser(), SLOT(updateKey(CSwordKey*)));
    connect(keyChooser(), SIGNAL(keyChanged(CSwordKey*)), m_treeChooser, SLOT(updateKey(CSwordKey*)));
}

/** Init the view */
void CBookReadWindow::initView() {
    QSplitter* splitter = new QSplitter(this);
    m_treeChooser = new CBookTreeChooser(modules(), history(), key(), splitter);
    setDisplayWidget(new BtHtmlReadDisplay(this, splitter));
    m_treeChooser->hide();

    // Create Navigation toolbar
    setMainToolBar( new QToolBar(this) );
    mainToolBar()->setAllowedAreas(Qt::TopToolBarArea);
    mainToolBar()->setFloatable(false);
    addToolBar(mainToolBar());
    setKeyChooser( CKeyChooser::createInstance(modules(), history(), key(), mainToolBar()) );

    // Create the Works toolbar
    setModuleChooserBar( new BtModuleChooserBar(this));
    moduleChooserBar()->setModules(getModuleList(), modules().first()->type(), this);
    addToolBar(moduleChooserBar());

    // Create the Tools toolbar
    setButtonsToolBar( new QToolBar(this) );
    buttonsToolBar()->setAllowedAreas(Qt::TopToolBarArea);
    buttonsToolBar()->setFloatable(false);
    addToolBar(buttonsToolBar());

    setCentralWidget( splitter );
    setWindowIcon(util::tool::getIconForModule(modules().first()));
}

void CBookReadWindow::initToolbars() {
    Q_ASSERT(m_treeAction);
    Q_ASSERT(m_actions.backInHistory);

    mainToolBar()->addAction(m_actions.backInHistory);
    mainToolBar()->addAction(m_actions.forwardInHistory);
    mainToolBar()->addWidget(keyChooser());

    // Tools toolbar
    buttonsToolBar()->addAction(m_treeAction);  // Tree
    m_treeAction->setChecked(false);
    BtDisplaySettingsButton* button = new BtDisplaySettingsButton(buttonsToolBar());
    setDisplaySettingsButton(button);
    buttonsToolBar()->addWidget(button);  // Display settings
    QAction *action = actionCollection()->action(CResMgr::displaywindows::general::search::actionName);
    if (action != nullptr) {
        buttonsToolBar()->addAction(action);  // Search
    }
}

void CBookReadWindow::setupMainWindowToolBars() {
    // Navigation toolbar
    btMainWindow()->navToolBar()->addAction(m_actions.backInHistory); //1st button
    btMainWindow()->navToolBar()->addAction(m_actions.forwardInHistory); //2nd button
    CKeyChooser* keyChooser = CKeyChooser::createInstance(modules(), history(), key(), btMainWindow()->navToolBar() );
    btMainWindow()->navToolBar()->addWidget(keyChooser);
    bool ok = connect(keyChooser, SIGNAL(keyChanged(CSwordKey*)), this, SLOT(lookupSwordKey(CSwordKey*)));
    Q_ASSERT(ok);
    ok = connect(this, SIGNAL(sigKeyChanged(CSwordKey*)), keyChooser, SLOT(updateKey(CSwordKey*)) );
    Q_ASSERT(ok);

    // Works toolbar
    btMainWindow()->worksToolBar()->setModules(getModuleList(), modules().first()->type(), this);

    // Tools toolbar
    btMainWindow()->toolsToolBar()->addAction(m_treeAction);  // Tree
    BtDisplaySettingsButton* button = new BtDisplaySettingsButton(buttonsToolBar());
    setDisplaySettingsButton(button);
    btMainWindow()->toolsToolBar()->addWidget(button);  // Display settings
    QAction *action = actionCollection()->action(CResMgr::displaywindows::general::search::actionName);
    if (action != nullptr) {
        btMainWindow()->toolsToolBar()->addAction(action);  // Search
    }
}

/** Is called when the action was executed to toggle the tree view. */
void CBookReadWindow::treeToggled() {
    if (m_treeAction->isChecked()) {
        m_treeChooser->doShow();
    }
    else {
        m_treeChooser->hide();
    }
}

/** Reimplementation to take care of the tree chooser. */
void CBookReadWindow::modulesChanged() {
    CLexiconReadWindow::modulesChanged();
    m_treeChooser->setModules(modules());
}

void CBookReadWindow::setupPopupMenu() {
    CLexiconReadWindow::setupPopupMenu();
}

void CBookReadWindow::reload(CSwordBackend::SetupChangedReason reason) {
    CLexiconReadWindow::reload(reason);
}
