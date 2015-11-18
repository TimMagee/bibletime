/*********
*
* This file is part of BibleTime's source code, http://www.bibletime.info/.
*
* Copyright 1999-2015 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License version 2.0.
*
**********/

#include "frontend/displaywindow/clexiconreadwindow.h"

#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QMenu>
#include <QDebug>

#include "backend/keys/cswordldkey.h"
#include "backend/keys/cswordkey.h"
#include "bibletime.h"
#include "bibletimeapp.h"
#include "frontend/cexportmanager.h"
#include "frontend/display/bthtmlreaddisplay.h"
#include "frontend/displaywindow/btactioncollection.h"
#include "frontend/displaywindow/bttoolbarpopupaction.h"
#include "frontend/displaywindow/btdisplaysettingsbutton.h"
#include "frontend/displaywindow/bttextwindowheader.h"
#include "frontend/displaywindow/btmodulechooserbar.h"
#include "frontend/keychooser/bthistory.h"
#include "frontend/keychooser/ckeychooser.h"
#include "util/directory.h"
#include "util/cresmgr.h"
#include "util/tool.h"


CLexiconReadWindow::CLexiconReadWindow(const QList<CSwordModuleInfo *> & moduleList, CMDIArea * parent)
        : CReadWindow(moduleList, parent) {
    moduleList.first();
    setKey( CSwordKey::createInstance(moduleList.first()) );
}

CLexiconReadWindow::~CLexiconReadWindow() {
}

void CLexiconReadWindow::insertKeyboardActions( BtActionCollection* const a ) {
    QAction* qaction;
    qaction = new QAction( tr("Next entry"), a);
    qaction->setShortcut(CResMgr::displaywindows::lexiconWindow::nextEntry::accel);
    a->addAction("nextEntry", qaction);

    qaction = new QAction( tr("Previous entry"), a);
    qaction->setShortcut( CResMgr::displaywindows::lexiconWindow::previousEntry::accel);
    a->addAction("previousEntry", qaction);

    qaction = new QAction(tr("Copy reference only"), a);
    a->addAction("copyReferenceOnly", qaction);

    qaction = new QAction(tr("Save entry as HTML"), a);
    a->addAction("saveHtml", qaction);

    qaction = new QAction(tr("Print reference only"), a);
    a->addAction("printReferenceOnly", qaction);

    qaction = new QAction(tr("Entry with text"), a);
    a->addAction("copyEntryWithText", qaction);

    qaction = new QAction(tr("Entry as plain text"), a);
    a->addAction("saveEntryAsPlain", qaction);

    qaction = new QAction(tr("Entry with text"), a);
    a->addAction("printEntryWithText", qaction);

    qaction = new QAction( /* QIcon(CResMgr::displaywindows::general::findStrongs::icon), */ tr("Strong's Search"), a);
    qaction->setShortcut(CResMgr::displaywindows::general::findStrongs::accel);
    a->addAction(CResMgr::displaywindows::general::findStrongs::actionName, qaction);
}

void CLexiconReadWindow::initActions() {
    BtActionCollection* ac = actionCollection();
    CReadWindow::initActions();
    CLexiconReadWindow::insertKeyboardActions(ac);

    QAction *qaction = ac->action(CResMgr::displaywindows::general::backInHistory::actionName);
    Q_ASSERT(qaction != nullptr);
    m_actions.backInHistory = dynamic_cast<BtToolBarPopupAction*>(qaction);
    Q_ASSERT(m_actions.backInHistory);
    addAction(m_actions.backInHistory);

    qaction = ac->action(CResMgr::displaywindows::general::forwardInHistory::actionName);
    Q_ASSERT(qaction != nullptr);
    m_actions.forwardInHistory = dynamic_cast<BtToolBarPopupAction*>(qaction);
    Q_ASSERT(m_actions.forwardInHistory);
    addAction(m_actions.forwardInHistory);

    qaction = ac->action("nextEntry");
    Q_ASSERT(qaction != nullptr);
    connect(qaction, SIGNAL(triggered()),
            this,    SLOT(nextEntry()));
    addAction(qaction);

    qaction = ac->action("previousEntry");
    Q_ASSERT(qaction != nullptr);
    connect(qaction, SIGNAL(triggered()),
            this,   SLOT(previousEntry()));
    addAction(qaction);

    m_actions.selectAll = ac->action("selectAll");
    Q_ASSERT(m_actions.selectAll != nullptr);

    m_actions.findText = ac->action("findText");
    Q_ASSERT(m_actions.findText != nullptr);

    m_actions.findStrongs = ac->action(CResMgr::displaywindows::general::findStrongs::actionName);
    Q_ASSERT(m_actions.findStrongs != nullptr);
    connect(m_actions.findStrongs, SIGNAL(triggered()),
            this,                  SLOT(openSearchStrongsDialog()) );
    addAction(m_actions.findStrongs);

    m_actions.copy.reference = ac->action("copyReferenceOnly");
    Q_ASSERT(m_actions.copy.reference != nullptr);
    connect(m_actions.copy.reference,            SIGNAL(triggered()),
            displayWidget()->connectionsProxy(), SLOT(copyAnchorOnly()));
    addAction(m_actions.copy.reference);

    m_actions.copy.entry = ac->action("copyEntryWithText");
    Q_ASSERT(m_actions.copy.entry != nullptr);
    connect(m_actions.copy.entry,                SIGNAL(triggered()),
            displayWidget()->connectionsProxy(), SLOT(copyAll()));
    addAction(m_actions.copy.entry);

    m_actions.copy.selectedText = ac->action("copySelectedText");
    Q_ASSERT(m_actions.copy.selectedText != nullptr);

    m_actions.save.entryAsPlain = new QAction(tr("Entry as plain text"), ac );
    connect(m_actions.save.entryAsPlain, SIGNAL(triggered()),
            this,                        SLOT(saveAsPlain()));
    addAction(m_actions.save.entryAsPlain);

    m_actions.save.entryAsHTML = ac->action("saveHtml");
    Q_ASSERT(m_actions.save.entryAsHTML != nullptr);
    connect(m_actions.save.entryAsHTML, SIGNAL(triggered()),
            this,                       SLOT(saveAsHTML()));
    addAction(m_actions.save.entryAsHTML);

    m_actions.print.reference = ac->action("printReferenceOnly");
    Q_ASSERT(m_actions.print.reference != nullptr);
    connect(m_actions.print.reference, SIGNAL(triggered()),
            this,                      SLOT(printAnchorWithText()));
    addAction(m_actions.print.reference);

    m_actions.print.entry = ac->action("printEntryWithText");
    Q_ASSERT(m_actions.print.entry != nullptr);
    connect(m_actions.print.entry, SIGNAL(triggered()),
            this,                  SLOT(printAll()));
    addAction(m_actions.print.entry);

    // init with the user defined settings
    ac->readShortcuts("Lexicon shortcuts");
}

/** No descriptions */
void CLexiconReadWindow::initConnections() {
    Q_ASSERT(keyChooser());

    connect(keyChooser(), SIGNAL(keyChanged(CSwordKey*)), this, SLOT(lookupSwordKey(CSwordKey*)));
    connect(history(), SIGNAL(historyChanged(bool, bool)), this, SLOT(slotUpdateHistoryButtons(bool, bool)));

    //connect the history actions to the right slots
    bool ok = connect(
                  m_actions.backInHistory->popupMenu(), SIGNAL(aboutToShow()),
                  this, SLOT(slotFillBackHistory())
              );
    Q_ASSERT(ok);
    ok = connect(
             m_actions.backInHistory->popupMenu(), SIGNAL(triggered(QAction*)),
             keyChooser()->history(), SLOT(move(QAction*))
         );
    Q_ASSERT(ok);
    ok = connect(
             m_actions.forwardInHistory->popupMenu(), SIGNAL(aboutToShow()),
             this, SLOT(slotFillForwardHistory())
         );
    Q_ASSERT(ok);
    ok = connect(
             m_actions.forwardInHistory->popupMenu(), SIGNAL(triggered(QAction*)),
             keyChooser()->history(), SLOT(move(QAction*))
         );
    Q_ASSERT(ok);

}

void CLexiconReadWindow::initView() {
    // Create display widget for this window
    setDisplayWidget(new BtHtmlReadDisplay(this, this));
    setCentralWidget( displayWidget()->view() );
    setWindowIcon(util::tool::getIconForModule(modules().first()));

    // Create the Navigation toolbar
    setMainToolBar( new QToolBar(this) );
    addToolBar(mainToolBar());

    // Create keychooser
    setKeyChooser( CKeyChooser::createInstance(modules(), history(), key(), mainToolBar()) );

    // Create the Works toolbar
    setModuleChooserBar( new BtModuleChooserBar(this));
    moduleChooserBar()->setModules(getModuleList(), modules().first()->type(), this);
    addToolBar(moduleChooserBar());

    // Create the Tools toolbar
    setButtonsToolBar( new QToolBar(this) );
    addToolBar(buttonsToolBar());

    // Create the Text Header toolbar
    addToolBarBreak();
    setHeaderBar(new QToolBar(this));
    addToolBar(headerBar());
}

void CLexiconReadWindow::initToolbars() {
    //Navigation toolbar
    Q_ASSERT(m_actions.backInHistory);
    mainToolBar()->addWidget(keyChooser());
    mainToolBar()->addAction(m_actions.backInHistory); //1st button
    mainToolBar()->addAction(m_actions.forwardInHistory); //2nd button

    //Tools toolbar
    QAction *action = actionCollection()->action(CResMgr::displaywindows::general::search::actionName);
    Q_ASSERT(action != nullptr);
    buttonsToolBar()->addAction(action);

    BtDisplaySettingsButton* button = new BtDisplaySettingsButton(buttonsToolBar());
    setDisplaySettingsButton(button);
    buttonsToolBar()->addWidget(button);

    // Text Header toolbar
    BtTextWindowHeader *h = new BtTextWindowHeader(modules().first()->type(), getModuleList(), this);
    headerBar()->addWidget(h);
}

void CLexiconReadWindow::setupMainWindowToolBars() {
    // Navigation toolbar
    CKeyChooser* keyChooser = CKeyChooser::createInstance(modules(), history(), key(), btMainWindow()->navToolBar() );
    btMainWindow()->navToolBar()->addWidget(keyChooser);
    bool ok = connect(keyChooser, SIGNAL(keyChanged(CSwordKey*)), this, SLOT(lookupSwordKey(CSwordKey*)));
    Q_ASSERT(ok);
    ok = connect(this, SIGNAL(sigKeyChanged(CSwordKey*)), keyChooser, SLOT(updateKey(CSwordKey*)) );
    Q_ASSERT(ok);
    btMainWindow()->navToolBar()->addAction(m_actions.backInHistory); //1st button
    btMainWindow()->navToolBar()->addAction(m_actions.forwardInHistory); //2nd button

    // Works toolbar
    btMainWindow()->worksToolBar()->setModules(getModuleList(), modules().first()->type(), this);

    // Tools toolbar
    QAction *action = actionCollection()->action(CResMgr::displaywindows::general::search::actionName);
    Q_ASSERT(action != nullptr);
    btMainWindow()->toolsToolBar()->addAction(action);
    BtDisplaySettingsButton* button = new BtDisplaySettingsButton(buttonsToolBar());
    setDisplaySettingsButton(button);
    btMainWindow()->toolsToolBar()->addWidget(button);
}

void CLexiconReadWindow::setupPopupMenu() {
    popup()->setTitle(tr("Lexicon window"));
    popup()->setIcon(util::tool::getIconForModule(modules().first()));
    popup()->addAction(m_actions.findText);
    popup()->addAction(m_actions.findStrongs);
    popup()->addAction(m_actions.selectAll);
    popup()->addSeparator();

    m_actions.copyMenu = new QMenu(tr("Copy..."), popup());

    m_actions.copyMenu->addAction(m_actions.copy.reference);
    m_actions.copyMenu->addAction(m_actions.copy.entry);
    m_actions.copyMenu->addSeparator();
    m_actions.copyMenu->addAction(m_actions.copy.selectedText);
    popup()->addMenu(m_actions.copyMenu);

    m_actions.saveMenu = new QMenu(
        tr("Save..."),
        popup()
    );
    m_actions.saveMenu->addAction(m_actions.save.entryAsPlain);
    m_actions.saveMenu->addAction(m_actions.save.entryAsHTML);

    // Save raw HTML action for debugging purposes
    if (btApp->debugMode()) {
        QAction* debugAction = new QAction("Raw HTML", this);
        QObject::connect(debugAction, SIGNAL(triggered()), this, SLOT(saveRawHTML()));
        m_actions.saveMenu->addAction(debugAction);
    } // end of Save Raw HTML

    popup()->addMenu(m_actions.saveMenu);

    m_actions.printMenu = new QMenu(
        tr("Print..."),
        popup()
    );
    m_actions.printMenu->addAction(m_actions.print.reference);
    m_actions.printMenu->addAction(m_actions.print.entry);
    popup()->addMenu(m_actions.printMenu);
}

/** Reimplemented. */
void CLexiconReadWindow::updatePopupMenu() {
    //enable the action depending on the supported module features

    CReadDisplay const & display =
            *static_cast<CReadDisplay *>(displayWidget());

    m_actions.findStrongs->setEnabled(!display.getCurrentNodeInfo().isNull());

    bool const hasActiveAnchor = display.hasActiveAnchor();
    m_actions.copy.reference->setEnabled(hasActiveAnchor);
    m_actions.copy.selectedText->setEnabled(display.hasSelection());

    m_actions.print.reference->setEnabled(hasActiveAnchor);
}

void CLexiconReadWindow::reload(CSwordBackend::SetupChangedReason reason) {
    CReadWindow::reload(reason);

    actionCollection()->readShortcuts("Lexicon shortcuts");
}

/** No descriptions */
void CLexiconReadWindow::nextEntry() {
    keyChooser()->setKey(ldKey()->NextEntry());
}

/** No descriptions */
void CLexiconReadWindow::previousEntry() {
    keyChooser()->setKey(ldKey()->PreviousEntry());
}

/** Reimplementation to return the right key. */
CSwordLDKey* CLexiconReadWindow::ldKey() {
    return dynamic_cast<CSwordLDKey*>(CDisplayWindow::key());
}

/** This function saves the entry as html using the CExportMgr class. */
void CLexiconReadWindow::saveAsHTML() {
    CExportManager mgr(true, tr("Saving"), filterOptions(), displayOptions());
    mgr.saveKey(key(), CExportManager::HTML, true);
}

/** Saving the raw HTML for debugging purposes */
void CLexiconReadWindow::saveRawHTML() {
    QString savefilename = QFileDialog::getSaveFileName();
    if (savefilename.isEmpty()) return;
    BtHtmlReadDisplay* disp = dynamic_cast<BtHtmlReadDisplay*>(displayWidget());
    if (disp) {
        QFile file(savefilename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "saveRawHTML: could not open file" << savefilename;
            return;
        }
        QString source = disp->getCurrentSource();
        file.write(source.toUtf8());
        file.close();
        file.flush();
    }
}

/** This function saves the entry as html using the CExportMgr class. */
void CLexiconReadWindow::saveAsPlain() {
    CExportManager mgr(true, tr("Saving"), filterOptions(), displayOptions());
    mgr.saveKey(key(), CExportManager::Text, true);
}

void CLexiconReadWindow::slotFillBackHistory() {
    QMenu* menu = m_actions.backInHistory->popupMenu();
    menu->clear();

    /// \todo take the history list and fill the menu
    QListIterator<QAction*> it(keyChooser()->history()->getBackList());
    while (it.hasNext()) {
        menu->addAction(it.next());
    }
}

void CLexiconReadWindow::slotFillForwardHistory() {
    QMenu* menu = m_actions.forwardInHistory->popupMenu();
    menu->clear();
    /// \todo take the history list and fill the menu using addAction
    QListIterator<QAction*> it(keyChooser()->history()->getFwList());
    while (it.hasNext()) {
        menu->addAction(it.next());
    }
}


void CLexiconReadWindow::slotUpdateHistoryButtons(bool backEnabled, bool fwEnabled) {
    Q_ASSERT(m_actions.backInHistory);
    Q_ASSERT(keyChooser());

    m_actions.backInHistory->setEnabled( backEnabled );
    m_actions.forwardInHistory->setEnabled( fwEnabled );
}
