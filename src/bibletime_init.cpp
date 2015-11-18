/*********
*
* This file is part of BibleTime's source code, http://www.bibletime.info/.
*
* Copyright 1999-2015 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License version 2.0.
*
**********/

#include "bibletime.h"

#include <QApplication>
#include <QDebug>
#include <QDockWidget>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QPointer>
#include <QSplitter>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include "backend/config/btconfig.h"
#include "backend/managers/btstringmgr.h"
#include "backend/managers/clanguagemgr.h"
#include "backend/managers/cswordbackend.h"
#include "bibletimeapp.h"
#include "frontend/btbookshelfdockwidget.h"
#include "frontend/btopenworkaction.h"
#include "frontend/cinfodisplay.h"
#include "frontend/cmdiarea.h"
#include "frontend/cprinter.h"
#include "frontend/display/btfindwidget.h"
#include "frontend/displaywindow/btactioncollection.h"
#include "frontend/displaywindow/btmodulechooserbar.h"
#include "frontend/bookmarks/cbookmarkindex.h"
#include "frontend/settingsdialogs/cdisplaysettings.h"
#include "util/cresmgr.h"
#include "util/directory.h"

// Sword includes:
#include <swlog.h>
#include <swmgr.h>

#ifdef BT_DEBUG
#include <QLabel>
#include <QMetaObject>
#include <QMutexLocker>
#include <QTimer>
#endif


using namespace InfoDisplay;

/**Initializes the view of this widget*/
void BibleTime::initView() {

    // Create menu and toolbar before the mdi area
    createMenuAndToolBar();

    createCentralWidget();

    m_bookshelfDock = new BtBookshelfDockWidget(this);
    addDockWidget(Qt::LeftDockWidgetArea, m_bookshelfDock);

    m_bookmarksDock = new QDockWidget(this);
    m_bookmarksDock->setObjectName("BookmarksDock");
    m_bookmarksPage = new CBookmarkIndex(nullptr);
    m_bookmarksDock->setWidget(m_bookmarksPage);
    addDockWidget(Qt::LeftDockWidgetArea, m_bookmarksDock);
    tabifyDockWidget(m_bookmarksDock, m_bookshelfDock);

    m_magDock = new QDockWidget(this);
    m_magDock->setObjectName("MagDock");
    m_infoDisplay = new CInfoDisplay(this);
    m_infoDisplay->resize(150, 150);
    m_magDock->setWidget(m_infoDisplay);
    addDockWidget(Qt::LeftDockWidgetArea, m_magDock);

    connect(m_bookshelfDock, SIGNAL(moduleHovered(CSwordModuleInfo*)),
            m_infoDisplay,   SLOT(setInfo(CSwordModuleInfo*)));

    m_mdi->setMinimumSize(100, 100);
    m_mdi->setFocusPolicy(Qt::ClickFocus);
}

QAction* BibleTime::initAction(QAction* action, QString text, QIcon const & icon,
                               QKeySequence accel, const QString& tooltip,
                               const QString& actionName, const char* slot) {
    action->setText(text);
    action->setIcon(icon);
    action->setShortcut(accel);
    if (tooltip != QString::null) action->setToolTip(tooltip);
    m_actionCollection->addAction(actionName, action);
    if (slot) QObject::connect( action, SIGNAL(triggered()), this, slot );
    return action;
}

// Creates QAction's for all actions that can have keyboard shortcuts
// Used in creating the main window and by the configuration dialog for setting shortcuts
void BibleTime::insertKeyboardActions( BtActionCollection* const a ) {
    QAction* action = new QAction(a);
    action->setText(tr("&Quit"));
    action->setIcon(CResMgr::mainMenu::window::quit::icon());
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
    action->setToolTip(tr("Quit BibleTime"));
    a->addAction("quit", action);

    action = new QAction(a);
    action->setText(tr("&Fullscreen mode"));
    action->setIcon(CResMgr::mainMenu::window::showFullscreen::icon());
    action->setShortcut(QKeySequence(CResMgr::mainMenu::window::showFullscreen::accel));
    action->setToolTip(tr("Toggle fullscreen mode of the main window"));
    a->addAction("toggleFullscreen", action);

    action = new QAction(a);
    action->setText(tr("&Show toolbar"));
    action->setShortcut(QKeySequence(Qt::Key_F6));
    a->addAction("showToolbar", action);

    action = new QAction(a);
    action->setText(tr("Search in &open works..."));
    action->setIcon(CResMgr::mainMenu::mainIndex::search::icon());
    action->setShortcut(QKeySequence(CResMgr::mainMenu::mainIndex::search::accel));
    action->setToolTip(tr("Search in all works that are currently open"));
    a->addAction("searchOpenWorks", action);

    action = new QAction(a);
    action->setText(tr("Search in standard &Bible..."));
    action->setIcon(CResMgr::mainMenu::mainIndex::searchdefaultbible::icon());
    action->setShortcut(QKeySequence(CResMgr::mainMenu::mainIndex::searchdefaultbible::accel));
    action->setToolTip(tr("Search in the standard Bible"));
    a->addAction("searchStdBible", action);

    action = new QAction(a);
    action->setText(tr("Save as &new session..."));
    action->setIcon(CResMgr::mainMenu::window::saveToNewProfile::icon());
    action->setShortcut(QKeySequence(CResMgr::mainMenu::window::saveToNewProfile::accel));
    action->setToolTip(tr("Create and save a new session"));
    a->addAction("saveNewSession", action);

    action = new QAction(a);
    action->setText(tr("&Manual mode"));
    action->setIcon(CResMgr::mainMenu::window::arrangementMode::manual::icon());
    action->setShortcut(QKeySequence(CResMgr::mainMenu::window::arrangementMode::manual::accel));
    action->setToolTip(tr("Manually arrange the open windows"));
    a->addAction("manualArrangement", action);

    action = new QAction(a);
    action->setText(tr("Auto-tile &vertically"));
    action->setIcon(CResMgr::mainMenu::window::arrangementMode::autoTileVertical::icon());
    action->setShortcut(QKeySequence(CResMgr::mainMenu::window::arrangementMode::autoTileVertical::accel));
    action->setToolTip(tr("Automatically tile the open windows vertically (arrange side by side)"));
    a->addAction("autoVertical", action);

    action = new QAction(a);
    action->setText(tr("Auto-tile &horizontally"));
    action->setIcon(CResMgr::mainMenu::window::arrangementMode::autoTileHorizontal::icon());
    action->setShortcut(QKeySequence(CResMgr::mainMenu::window::arrangementMode::autoTileHorizontal::accel));
    action->setToolTip(tr("Automatically tile the open windows horizontally (arrange on top of each other)"));
    a->addAction("autoHorizontal", action);

    action = new QAction(a);
    action->setText(tr("Auto-&tile"));
    action->setIcon(CResMgr::mainMenu::window::arrangementMode::autoTile::icon());
    action->setShortcut(QKeySequence(CResMgr::mainMenu::window::arrangementMode::autoTile::accel));
    action->setToolTip(tr("Automatically tile the open windows"));
    a->addAction("autoTile", action);

    action = new QAction(a);
    action->setText(tr("Ta&bbed"));
    action->setIcon(CResMgr::mainMenu::window::arrangementMode::autoTabbed::icon());
    action->setShortcut(QKeySequence(CResMgr::mainMenu::window::arrangementMode::autoTabbed::accel));
    action->setToolTip(tr("Automatically tab the open windows"));
    a->addAction("autoTabbed", action);

    action = new QAction(a);
    action->setText(tr("Auto-&cascade"));
    action->setIcon(CResMgr::mainMenu::window::arrangementMode::autoCascade::icon());
    action->setShortcut(QKeySequence(CResMgr::mainMenu::window::arrangementMode::autoCascade::accel));
    action->setToolTip(tr("Automatically cascade the open windows"));
    a->addAction("autoCascade", action);

    action = new QAction(a);
    action->setText(tr("&Cascade"));
    action->setIcon(CResMgr::mainMenu::window::cascade::icon());
    action->setShortcut(QKeySequence(CResMgr::mainMenu::window::cascade::accel));
    action->setToolTip(tr("Cascade the open windows"));
    a->addAction("cascade", action);

    action = new QAction(a);
    action->setText(tr("&Tile"));
    action->setIcon(CResMgr::mainMenu::window::tile::icon());
    action->setShortcut(QKeySequence(CResMgr::mainMenu::window::tile::accel));
    action->setToolTip(tr("Tile the open windows"));
    a->addAction("tile", action);

    action = new QAction(a);
    action->setText(tr("Tile &vertically"));
    action->setIcon(CResMgr::mainMenu::window::tileVertical::icon());
    action->setShortcut(QKeySequence(CResMgr::mainMenu::window::tileVertical::accel));
    action->setToolTip(tr("Vertically tile (arrange side by side) the open windows"));
    a->addAction("tileVertically", action);

    action = new QAction(a);
    action->setText(tr("Tile &horizontally"));
    action->setIcon(CResMgr::mainMenu::window::tileHorizontal::icon());
    action->setShortcut(QKeySequence(CResMgr::mainMenu::window::tileHorizontal::accel));
    action->setToolTip(tr("Horizontally tile (arrange on top of each other) the open windows"));
    a->addAction("tileHorizontally", action);

    action = new QAction(a);
    action->setText(tr("Close &window"));
    action->setIcon(CResMgr::mainMenu::window::close::icon());
    action->setShortcut(QKeySequence(CResMgr::mainMenu::window::close::accel));
    action->setToolTip(tr("Close the current open window"));
    a->addAction("closeWindow", action);

    action = new QAction(a);
    action->setText(tr("Cl&ose all windows"));
    action->setIcon(CResMgr::mainMenu::window::closeAll::icon());
    action->setShortcut(QKeySequence(CResMgr::mainMenu::window::closeAll::accel));
    action->setToolTip(tr("Close all open windows inside BibleTime"));
    a->addAction("closeAllWindows", action);

    action = new QAction(a);
    action->setText(tr("&Configure BibleTime..."));
    action->setIcon(CResMgr::mainMenu::settings::configureDialog::icon());
    action->setToolTip(tr("Set BibleTime's preferences"));
    a->addAction("setPreferences", action);

    action = new QAction(a);
    action->setText(tr("Bookshelf &Manager..."));
    action->setIcon(CResMgr::mainMenu::settings::swordSetupDialog::icon());
    action->setShortcut(QKeySequence(CResMgr::mainMenu::settings::swordSetupDialog::accel));
    action->setToolTip(tr("Configure your bookshelf and install/update/remove/index works"));
    a->addAction("bookshelfManager", action);

    action = new QAction(a);
    action->setText(tr("&Handbook"));
    action->setIcon(CResMgr::mainMenu::help::handbook::icon());
    action->setShortcut(QKeySequence(CResMgr::mainMenu::help::handbook::accel));
    action->setToolTip(tr("Open BibleTime's handbook"));
    a->addAction("openHandbook", action);

    action = new QAction(a);
    action->setText(tr("&Bible Study Howto"));
    action->setIcon(CResMgr::mainMenu::help::bibleStudyHowTo::icon());
    action->setShortcut(QKeySequence(CResMgr::mainMenu::help::bibleStudyHowTo::accel));
    action->setToolTip(tr("Open the Bible study HowTo included with BibleTime.<br/>This HowTo is an introduction on how to study the Bible in an efficient way."));
    a->addAction("bibleStudyHowto", action);

    action = new QAction(a);
    action->setText(tr("&About BibleTime"));
    action->setIcon(CResMgr::mainMenu::help::aboutBibleTime::icon());
    action->setToolTip(tr("Information about the BibleTime program"));
    a->addAction("aboutBibleTime", action);

    action = new QAction(a);
    action->setText(tr("&Tip of the day..."));
    action->setIcon(CResMgr::mainMenu::help::tipOfTheDay::icon());
    action->setShortcut(QKeySequence(CResMgr::mainMenu::help::tipOfTheDay::accel));
    action->setToolTip(tr("Show tips about BibleTime"));
    a->addAction("tipOfTheDay", action);

    action = new QAction(a);
    a->addAction("showToolbarsInTextWindows", action);

    action = new QAction(a);
    a->addAction("showNavigation", action);

    action = new QAction(a);
    a->addAction("showWorks", action);

    action = new QAction(a);
    a->addAction("showTools", action);

    action = new QAction(a);
    a->addAction("showFormat", action);

    action = new QAction(a);
    a->addAction("showParallelTextHeaders", action);

    action = new QAction(a);
    a->addAction("showBookshelf", action);

    action = new QAction(a);
    a->addAction("showBookmarks", action);

    action = new QAction(a);
    a->addAction("showMag", action);

    retranslateUiActions(a);
}

static QToolBar* createToolBar(const QString& name, QWidget* parent, bool visible) {
    QToolBar* bar = new QToolBar(parent);
    bar->setObjectName(name);
    bar->setFloatable(false);
    bar->setMovable(true);
    bar->setVisible(visible);
    return bar;
}

void BibleTime::clearMdiToolBars() {
    // Clear main window toolbars
    m_navToolBar->clear();
    m_worksToolBar->clear();
    m_toolsToolBar->clear();
    m_formatToolBar->clear();
}

void BibleTime::createMenuAndToolBar()
{
    // Create menubar
    menuBar();

    m_mainToolBar = createToolBar("MainToolBar", this, true);
    addToolBar(m_mainToolBar);

    // Set visibility of main window toolbars based on config
    bool visible = ! btConfig().sessionValue<bool>("GUI/showToolbarsInEachWindow", true);

    m_navToolBar = createToolBar("NavToolBar", this, visible);
    addToolBar(m_navToolBar);

    m_worksToolBar = new BtModuleChooserBar(this);
    m_worksToolBar->setObjectName("WorksToolBar");
    m_worksToolBar->setVisible(visible);
    addToolBar(m_worksToolBar);

    m_toolsToolBar = createToolBar("ToolsToolBar", this, visible);
    addToolBar(m_toolsToolBar);

    m_formatToolBar = createToolBar("FormatToolBar", this, visible);
    addToolBar(m_formatToolBar);
}

void BibleTime::createCentralWidget()
{
    m_mdi = new CMDIArea(this);
    m_findWidget = new BtFindWidget(this);
    m_findWidget->setVisible(false);

    QVBoxLayout* layout = new QVBoxLayout();
    QMargins margins(0, 0, 0, 0);
    layout->setContentsMargins(margins);
    layout->addWidget(m_mdi);
    layout->addWidget(m_findWidget);

    QWidget* widget = new QWidget(this);
    widget->setLayout(layout);
    setCentralWidget(widget);

    bool ok = connect(m_findWidget, SIGNAL(findNext(const QString&,bool)),
        m_mdi, SLOT(findNextTextInActiveWindow(const QString &, bool)));
    Q_ASSERT(ok);

    ok = connect(m_findWidget, SIGNAL(findPrevious(const QString&,bool)),
        m_mdi, SLOT(findPreviousTextInActiveWindow(const QString &, bool)));
    Q_ASSERT(ok);
    ok = connect(m_findWidget, SIGNAL(highlightText(const QString&,bool)),
        m_mdi, SLOT(highlightTextInActiveWindow(const QString &, bool)));
    Q_ASSERT(ok);

    ok = connect(m_mdi, SIGNAL(subWindowActivated(QMdiSubWindow*)),
            this, SLOT(slotActiveWindowChanged(QMdiSubWindow*)));
    Q_ASSERT(ok);
}

/** Initializes the action objects of the GUI */
void BibleTime::initActions() {
    m_actionCollection = new BtActionCollection(this);
    insertKeyboardActions(m_actionCollection);

    // Create the window to signal mapper and connect it up:
    m_windowMapper = new QSignalMapper(this);
    connect(m_windowMapper, SIGNAL(mapped(QWidget*)),
            this,           SLOT(slotSetActiveSubWindow(QWidget*)));

    // File menu actions:
    m_openWorkAction = new BtOpenWorkAction("GUI/mainWindow/openWorkAction/grouping", this);
    Q_ASSERT(m_openWorkAction != nullptr);
    connect(m_openWorkAction, SIGNAL(triggered(CSwordModuleInfo*)),
            this,             SLOT(createReadDisplayWindow(CSwordModuleInfo*)));

    m_quitAction = m_actionCollection->action("quit");
    m_quitAction->setMenuRole(QAction::QuitRole);
    Q_ASSERT(m_quitAction != nullptr);
    connect(m_quitAction, SIGNAL(triggered()),
            this,         SLOT(quit()));


    // View menu actions:
    m_windowFullscreenAction = m_actionCollection->action("toggleFullscreen");
    Q_ASSERT(m_windowFullscreenAction != nullptr);
    m_windowFullscreenAction->setCheckable(true);
    connect(m_windowFullscreenAction, SIGNAL(triggered()),
            this,                     SLOT(toggleFullscreen()));

    // Special case these actions, overwrite those already in collection
    m_showBookshelfAction = m_bookshelfDock->toggleViewAction();
    m_showBookshelfAction->setIcon(CResMgr::mainMenu::view::showBookshelf::icon());
    m_showBookshelfAction->setToolTip(tr("Toggle visibility of the bookshelf window"));
    m_actionCollection->addAction("showBookshelf", m_showBookshelfAction);
    m_showBookmarksAction = m_bookmarksDock->toggleViewAction();
    m_showBookmarksAction->setIcon(CResMgr::mainMenu::view::showBookmarks::icon());
    m_showBookmarksAction->setToolTip(tr("Toggle visibility of the bookmarks window"));
    m_actionCollection->addAction("showBookmarks", m_showBookmarksAction);
    m_showMagAction = m_magDock->toggleViewAction();
    m_showMagAction->setIcon(CResMgr::mainMenu::view::showMag::icon());
    m_showMagAction->setToolTip(tr("Toggle visibility of the mag window"));
    m_actionCollection->addAction("showMag", m_showMagAction);

    m_showTextAreaHeadersAction = m_actionCollection->action("showParallelTextHeaders");
    Q_ASSERT(m_showTextAreaHeadersAction != nullptr);
    m_showTextAreaHeadersAction->setCheckable(true);
    m_showTextAreaHeadersAction->setChecked(btConfig().sessionValue<bool>("GUI/showTextWindowHeaders", true));
    connect(m_showTextAreaHeadersAction, SIGNAL(toggled(bool)),
            this,                        SLOT(slotToggleTextWindowHeader()));

    m_showMainWindowToolbarAction = m_actionCollection->action("showToolbar");
    Q_ASSERT(m_showMainWindowToolbarAction != nullptr);
    m_showMainWindowToolbarAction->setCheckable(true);
    m_showMainWindowToolbarAction->setChecked(btConfig().sessionValue<bool>("GUI/showMainToolbar", true));
    connect( m_showMainWindowToolbarAction, SIGNAL(triggered()),
            this,                SLOT(slotToggleMainToolbar()));

    m_showTextWindowNavigationAction = m_actionCollection->action("showNavigation");
    Q_ASSERT(m_showTextWindowNavigationAction != nullptr);
    m_showTextWindowNavigationAction->setCheckable(true);
    m_showTextWindowNavigationAction->setChecked(btConfig().sessionValue<bool>("GUI/showTextWindowNavigator", true));
    connect(m_showTextWindowNavigationAction, SIGNAL(toggled(bool)),
            this,                             SLOT(slotToggleNavigatorToolbar()));

    m_showTextWindowModuleChooserAction = m_actionCollection->action("showWorks");
    Q_ASSERT(m_showTextWindowModuleChooserAction != nullptr);
    m_showTextWindowModuleChooserAction->setCheckable(true);
    m_showTextWindowModuleChooserAction->setChecked(btConfig().sessionValue<bool>("GUI/showTextWindowModuleSelectorButtons", true));
    connect(m_showTextWindowModuleChooserAction, SIGNAL(toggled(bool)),
            this,                                SLOT(slotToggleWorksToolbar()));

    m_showTextWindowToolButtonsAction = m_actionCollection->action("showTools");
    Q_ASSERT(m_showTextWindowToolButtonsAction != nullptr);
    m_showTextWindowToolButtonsAction->setCheckable(true);
    m_showTextWindowToolButtonsAction->setChecked(btConfig().sessionValue<bool>("GUI/showTextWindowToolButtons", true));
    connect(m_showTextWindowToolButtonsAction, SIGNAL(toggled(bool)),
            this,                              SLOT(slotToggleToolsToolbar()));

    m_showFormatToolbarAction = m_actionCollection->action("showFormat");
    Q_ASSERT(m_showFormatToolbarAction != nullptr);
    m_showFormatToolbarAction->setCheckable(true);
    m_showFormatToolbarAction->setChecked(btConfig().sessionValue<bool>("GUI/showFormatToolbarButtons", true));
    bool ok = connect(m_showFormatToolbarAction, SIGNAL(toggled(bool)),
                      this,                      SLOT(slotToggleFormatToolbar()));
    Q_ASSERT(ok);

    m_toolbarsInEachWindow = m_actionCollection->action("showToolbarsInTextWindows");
    Q_ASSERT(m_toolbarsInEachWindow != nullptr);
    m_toolbarsInEachWindow->setCheckable(true);
    m_toolbarsInEachWindow->setChecked(btConfig().sessionValue<bool>("GUI/showToolbarsInEachWindow", true));
    ok = connect(m_toolbarsInEachWindow, SIGNAL(toggled(bool)),
                 this,                   SLOT(slotToggleToolBarsInEachWindow()));
    Q_ASSERT(ok);

    // Search menu actions:
    m_searchOpenWorksAction = m_actionCollection->action("searchOpenWorks");
    Q_ASSERT(m_searchOpenWorksAction != nullptr);
    connect(m_searchOpenWorksAction, SIGNAL(triggered()),
            this,                    SLOT(slotSearchModules()));

    m_searchStandardBibleAction = m_actionCollection->action("searchStdBible");
    Q_ASSERT(m_searchStandardBibleAction != nullptr);
    connect(m_searchStandardBibleAction, SIGNAL(triggered()),
            this,                        SLOT(slotSearchDefaultBible()));

    // Window menu actions:
    m_windowCloseAction = m_actionCollection->action("closeWindow");
    Q_ASSERT(m_windowCloseAction != nullptr);
    connect(m_windowCloseAction, SIGNAL(triggered()),
            m_mdi,                SLOT(closeActiveSubWindow()));

    m_windowCloseAllAction = m_actionCollection->action("closeAllWindows");
    Q_ASSERT(m_windowCloseAllAction != nullptr);
    connect(m_windowCloseAllAction, SIGNAL(triggered()),
            m_mdi,                   SLOT(closeAllSubWindows()));

    m_windowCascadeAction = m_actionCollection->action("cascade");
    Q_ASSERT(m_windowCascadeAction != nullptr);
    connect(m_windowCascadeAction, SIGNAL(triggered()),
            this,                   SLOT(slotCascade()));

    m_windowTileAction = m_actionCollection->action("tile");
    Q_ASSERT(m_windowTileAction != nullptr);
    connect(m_windowTileAction, SIGNAL(triggered()),
            this,                SLOT(slotTile()));

    m_windowTileVerticalAction = m_actionCollection->action("tileVertically");
    Q_ASSERT(m_windowTileVerticalAction != nullptr);
    connect(m_windowTileVerticalAction, SIGNAL(triggered()),
            this,                        SLOT(slotTileVertical()));

    m_windowTileHorizontalAction = m_actionCollection->action("tileHorizontally");
    Q_ASSERT(m_windowTileHorizontalAction != nullptr);
    connect(m_windowTileHorizontalAction, SIGNAL(triggered()),
            this,                          SLOT(slotTileHorizontal()));

    alignmentMode alignment = btConfig().sessionValue<alignmentMode>("GUI/alignmentMode", autoTileVertical);

    m_windowManualModeAction = m_actionCollection->action("manualArrangement");
    Q_ASSERT(m_windowManualModeAction != nullptr);
    m_windowManualModeAction->setCheckable(true);

    m_windowAutoTabbedAction = m_actionCollection->action("autoTabbed");
    Q_ASSERT(m_windowAutoTabbedAction != nullptr);
    m_windowAutoTabbedAction->setCheckable(true);

    //: Vertical tiling means that windows are vertical, placed side by side
    m_windowAutoTileVerticalAction = m_actionCollection->action("autoVertical");
    Q_ASSERT(m_windowAutoTileVerticalAction != nullptr);
    m_windowAutoTileVerticalAction->setCheckable(true);

    //: Horizontal tiling means that windows are horizontal, placed on top of each other
    m_windowAutoTileHorizontalAction = m_actionCollection->action("autoHorizontal");
    Q_ASSERT(m_windowAutoTileHorizontalAction != nullptr);
    m_windowAutoTileHorizontalAction->setCheckable(true);

    m_windowAutoTileAction = m_actionCollection->action("autoTile");
    Q_ASSERT(m_windowAutoTileAction != nullptr);
    m_windowAutoTileAction->setCheckable(true);

    m_windowAutoCascadeAction = m_actionCollection->action("autoCascade");
    Q_ASSERT(m_windowAutoCascadeAction != nullptr);
    m_windowAutoCascadeAction->setCheckable(true);

    /*
     * All actions related to arrangement modes have to be initialized before calling a slot on them,
     * thus we call them afterwards now.
     */
    QAction * alignmentAction;
    switch (alignment) {
        case autoTabbed:
            alignmentAction = m_windowAutoTabbedAction; break;
        case autoTileVertical:
            alignmentAction = m_windowAutoTileVerticalAction; break;
        case autoTileHorizontal:
            alignmentAction = m_windowAutoTileHorizontalAction; break;
        case autoTile:
            alignmentAction = m_windowAutoTileAction; break;
        case autoCascade:
            alignmentAction = m_windowAutoCascadeAction; break;
        case manual:
        default:
            alignmentAction = m_windowManualModeAction; break;
    }
    alignmentAction->setChecked(true);
    slotUpdateWindowArrangementActions(alignmentAction);

    m_windowSaveToNewProfileAction = m_actionCollection->action("saveNewSession");
    Q_ASSERT(m_windowSaveToNewProfileAction != nullptr);
    connect(m_windowSaveToNewProfileAction, SIGNAL(triggered()),
            this,                            SLOT(saveToNewProfile()));

    m_setPreferencesAction = m_actionCollection->action("setPreferences");
    Q_ASSERT(m_setPreferencesAction != nullptr);
    m_setPreferencesAction->setMenuRole( QAction::PreferencesRole );
    connect(m_setPreferencesAction, SIGNAL(triggered()),
            this,                   SLOT(slotSettingsOptions()));

    m_bookshelfManagerAction = m_actionCollection->action("bookshelfManager");
    Q_ASSERT(m_bookshelfManagerAction != nullptr);
    m_bookshelfManagerAction->setMenuRole( QAction::ApplicationSpecificRole );
    connect(m_bookshelfManagerAction, SIGNAL(triggered()),
            this,                     SLOT(slotSwordSetupDialog()));

    m_openHandbookAction = m_actionCollection->action("openHandbook");
    Q_ASSERT(m_openHandbookAction != nullptr);
    connect(m_openHandbookAction, SIGNAL(triggered()),
            this,                 SLOT(openOnlineHelp_Handbook()));

    m_bibleStudyHowtoAction = m_actionCollection->action("bibleStudyHowto");
    Q_ASSERT(m_bibleStudyHowtoAction != nullptr);
    connect(m_bibleStudyHowtoAction, SIGNAL(triggered()),
            this,                    SLOT(openOnlineHelp_Howto()));

    m_aboutBibleTimeAction = m_actionCollection->action("aboutBibleTime");
    Q_ASSERT(m_aboutBibleTimeAction != nullptr);
    m_aboutBibleTimeAction->setMenuRole( QAction::AboutRole );
    connect(m_aboutBibleTimeAction,  SIGNAL(triggered()),
            this,                    SLOT(slotOpenAboutDialog()) );

    m_tipOfTheDayAction = m_actionCollection->action("tipOfTheDay");
    Q_ASSERT(m_tipOfTheDayAction != nullptr);
    connect(m_tipOfTheDayAction,  SIGNAL(triggered()),
            this,                    SLOT(slotOpenTipDialog()) );

    #ifdef BT_DEBUG
    m_debugWidgetAction = new QAction(this);
    m_debugWidgetAction->setCheckable(true);
    connect(m_debugWidgetAction, SIGNAL(triggered(bool)),
            this,                 SLOT(slotShowDebugWindow(bool)));
    #endif

    retranslateUiActions(m_actionCollection);
}

void BibleTime::initMenubar() {
    // File menu:
    m_fileMenu = new QMenu(this);
    m_fileMenu->addAction(m_openWorkAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_quitAction);
    menuBar()->addMenu(m_fileMenu);

    // View menu:
    m_viewMenu = new QMenu(this);
    m_viewMenu->addAction(m_windowFullscreenAction);
    m_viewMenu->addAction(m_showBookshelfAction);
    m_viewMenu->addAction(m_showBookmarksAction);
    m_viewMenu->addAction(m_showMagAction);
    m_viewMenu->addAction(m_showTextAreaHeadersAction);
    m_viewMenu->addSeparator();
    m_toolBarsMenu = new QMenu(this);
    m_toolBarsMenu->addAction( m_showMainWindowToolbarAction);
    m_toolBarsMenu->addAction(m_showTextWindowNavigationAction);
    m_toolBarsMenu->addAction(m_showTextWindowModuleChooserAction);
    m_toolBarsMenu->addAction(m_showTextWindowToolButtonsAction);
    m_toolBarsMenu->addAction(m_showFormatToolbarAction);
    m_toolBarsMenu->addSeparator();
    m_toolBarsMenu->addAction(m_toolbarsInEachWindow);
    m_viewMenu->addMenu(m_toolBarsMenu);
    menuBar()->addMenu(m_viewMenu);

    // Search menu:
    m_searchMenu = new QMenu(this);
    m_searchMenu->addAction(m_searchOpenWorksAction);
    m_searchMenu->addAction(m_searchStandardBibleAction);
    menuBar()->addMenu(m_searchMenu);

    // Window menu:
    m_windowMenu = new QMenu(this);
    m_openWindowsMenu = new QMenu(this);
    QObject::connect(m_openWindowsMenu, SIGNAL(aboutToShow()),
                     this,              SLOT(slotOpenWindowsMenuAboutToShow()));
    m_windowMenu->addMenu(m_openWindowsMenu);
    m_windowMenu->addAction(m_windowCloseAction);
    m_windowMenu->addAction(m_windowCloseAllAction);
    m_windowMenu->addSeparator();
    m_windowMenu->addAction(m_windowCascadeAction);
    m_windowMenu->addAction(m_windowTileAction);
    m_windowMenu->addAction(m_windowTileVerticalAction);
    m_windowMenu->addAction(m_windowTileHorizontalAction);
    m_windowArrangementMenu = new QMenu(this);
    m_windowArrangementActionGroup = new QActionGroup(m_windowArrangementMenu);
    m_windowArrangementMenu->addAction(m_windowManualModeAction);
    m_windowArrangementActionGroup->addAction(m_windowManualModeAction);
    m_windowArrangementMenu->addAction(m_windowAutoTabbedAction);
    m_windowArrangementActionGroup->addAction(m_windowAutoTabbedAction);
    m_windowArrangementMenu->addAction(m_windowAutoTileVerticalAction);
    m_windowArrangementActionGroup->addAction(m_windowAutoTileVerticalAction);
    m_windowArrangementMenu->addAction(m_windowAutoTileHorizontalAction);
    m_windowArrangementActionGroup->addAction(m_windowAutoTileHorizontalAction);
    m_windowArrangementMenu->addAction(m_windowAutoTileAction);
    m_windowArrangementActionGroup->addAction(m_windowAutoTileAction);
    m_windowArrangementMenu->addAction(m_windowAutoCascadeAction);
    m_windowArrangementActionGroup->addAction(m_windowAutoCascadeAction);
    connect(m_windowArrangementActionGroup, SIGNAL(triggered(QAction *)),
            this,                           SLOT(slotUpdateWindowArrangementActions(QAction *)));

    m_windowMenu->addMenu(m_windowArrangementMenu);
    m_windowMenu->addSeparator();
    m_windowMenu->addAction(m_windowSaveToNewProfileAction);
    m_windowLoadProfileMenu = new QMenu(this);
    m_windowLoadProfileActionGroup = new QActionGroup(m_windowLoadProfileMenu);
    m_windowMenu->addMenu(m_windowLoadProfileMenu);
    m_windowDeleteProfileMenu = new QMenu(this);
    m_windowMenu->addMenu(m_windowDeleteProfileMenu);
    connect(m_windowLoadProfileMenu, SIGNAL(triggered(QAction*)),
            this,                    SLOT(loadProfile(QAction*)));
    connect(m_windowDeleteProfileMenu, SIGNAL(triggered(QAction*)),
            this,                      SLOT(deleteProfile(QAction*)));
    refreshProfileMenus();
    menuBar()->addMenu(m_windowMenu);
    connect(m_windowMenu, SIGNAL(aboutToShow()),
            this,         SLOT(slotWindowMenuAboutToShow()));

    #ifndef Q_OS_MAC
    m_settingsMenu = new QMenu(this);
    m_settingsMenu->addAction(m_setPreferencesAction);
    m_settingsMenu->addSeparator();
    m_settingsMenu->addAction(m_bookshelfManagerAction);
    menuBar()->addMenu(m_settingsMenu);
    #else
    // On MAC OS, the settings actions will be moved to a system menu item.
    // Therefore the settings menu would be empty, so we do not show it.
    m_fileMenu->addAction(m_setPreferencesAction);
    m_fileMenu->addAction(m_bookshelfManagerAction);
    #endif

    // Help menu:
    m_helpMenu = new QMenu(this);
    m_helpMenu->addAction(m_openHandbookAction);
    m_helpMenu->addAction(m_bibleStudyHowtoAction);
    m_helpMenu->addAction(m_tipOfTheDayAction);
    m_helpMenu->addSeparator();
    m_helpMenu->addAction(m_aboutBibleTimeAction);
    #ifdef BT_DEBUG
    m_helpMenu->addSeparator();
    m_helpMenu->addAction(m_debugWidgetAction);
    #endif
    menuBar()->addMenu(m_helpMenu);
}

void BibleTime::initToolbars() {
    QToolButton *openWorkButton = new QToolButton(this);
    openWorkButton->setDefaultAction(m_openWorkAction);
    openWorkButton->setPopupMode(QToolButton::InstantPopup);
    m_mainToolBar->addWidget(openWorkButton);

    m_mainToolBar->addAction(m_windowFullscreenAction);
    QAction *a = m_actionCollection->action("showBookshelf");
    Q_ASSERT(a != nullptr);
    m_mainToolBar->addAction(a);
    a = m_actionCollection->action("showBookmarks");
    Q_ASSERT(a != nullptr);
    m_mainToolBar->addAction(a);
    a = m_actionCollection->action("showMag");
    Q_ASSERT(a != nullptr);
    m_mainToolBar->addAction(a);
    m_mainToolBar->addAction(m_searchOpenWorksAction);
    m_mainToolBar->addAction(m_openHandbookAction);
}

void BibleTime::retranslateUi() {
    m_bookmarksDock->setWindowTitle(tr("Bookmarks"));
    m_magDock->setWindowTitle(tr("Mag"));
    m_mainToolBar->setWindowTitle(tr("Main toolbar"));
    m_navToolBar->setWindowTitle(tr("Navigation toolbar"));
    m_worksToolBar->setWindowTitle(tr("Works toolbar"));
    m_toolsToolBar->setWindowTitle(tr("Tools toolbar"));
    m_formatToolBar->setWindowTitle(tr("Format toolbar"));

    m_fileMenu->setTitle(tr("&File"));
    m_viewMenu->setTitle(tr("&View"));
    m_toolBarsMenu->setTitle(tr("Toolbars"));

    m_searchMenu->setTitle(tr("&Search"));
    m_windowMenu->setTitle(tr("&Window"));
    m_openWindowsMenu->setTitle(tr("O&pen windows"));
    m_windowArrangementMenu->setTitle(tr("&Arrangement mode"));
    m_windowLoadProfileMenu->setTitle(tr("Sw&itch session"));
    m_windowDeleteProfileMenu->setTitle(tr("&Delete session"));

    #ifndef Q_OS_MAC
    // This item is not present on Mac OS
    m_settingsMenu->setTitle(tr("Se&ttings"));
    #endif

    m_helpMenu->setTitle(tr("&Help"));

    #ifdef BT_DEBUG
    m_debugWidgetAction->setText(tr("Show \"Whats this widget\" dialog"));
    #endif

    retranslateUiActions(m_actionCollection);
}

/** retranslation for actions used in this class
*   This is called for two different collections of actions
*   One set is for the actual use in the menus, etc
*   The second is used during the use of the configuration shortcut editor
*/
void BibleTime::retranslateUiActions(BtActionCollection* ac) {
    QAction *a = ac->action("showToolbarsInTextWindows");
    Q_ASSERT(a != nullptr);
    a->setText(tr("Show toolbars in text windows"));
    a = ac->action("showToolbar");
    Q_ASSERT(a != nullptr);
    a->setText(tr("Show main toolbar"));
    a = ac->action("showNavigation");
    Q_ASSERT(a != nullptr);
    a->setText(tr("Show navigation bar"));
    a = ac->action("showWorks");
    Q_ASSERT(a != nullptr);
    a->setText(tr("Show works toolbar"));
    a = ac->action("showTools");
    Q_ASSERT(a != nullptr);
    a->setText(tr("Show tools toolbar"));
    a = ac->action("showFormat");
    Q_ASSERT(a != nullptr);
    a->setText(tr("Show formatting toolbar"));
    a = ac->action("showBookshelf");
    Q_ASSERT(a != nullptr);
    a->setText(tr("Show bookshelf"));
    a = ac->action("showBookmarks");
    Q_ASSERT(a != nullptr);
    a->setText(tr("Show bookmarks"));
    a = ac->action("showMag");
    Q_ASSERT(a != nullptr);
    a->setText(tr("Show mag"));
    a = ac->action("showParallelTextHeaders");
    Q_ASSERT(a != nullptr);
    a->setText(tr("Show parallel text headers"));
}

/** Initializes the SIGNAL / SLOT connections */
void BibleTime::initConnections() {
    // Bookmarks page connections:
    connect(m_bookmarksPage, SIGNAL(createReadDisplayWindow(QList<CSwordModuleInfo*>, const QString&)),
            this,            SLOT(createReadDisplayWindow(QList<CSwordModuleInfo*>, const QString&)));

    // Bookshelf dock connections:
    connect(m_bookshelfDock, SIGNAL(moduleOpenTriggered(CSwordModuleInfo*)),
            this, SLOT(createReadDisplayWindow(CSwordModuleInfo*)));
    connect(m_bookshelfDock, SIGNAL(moduleSearchTriggered(CSwordModuleInfo*)),
            this, SLOT(searchInModule(CSwordModuleInfo*)));
    connect(m_bookshelfDock, SIGNAL(moduleEditPlainTriggered(CSwordModuleInfo*)),
            this, SLOT(moduleEditPlain(CSwordModuleInfo*)));
    connect(m_bookshelfDock, SIGNAL(moduleEditHtmlTriggered(CSwordModuleInfo*)),
            this, SLOT(moduleEditHtml(CSwordModuleInfo*)));
    connect(m_bookshelfDock, SIGNAL(moduleUnlockTriggered(CSwordModuleInfo*)),
            this, SLOT(slotModuleUnlock(CSwordModuleInfo*)));
    connect(m_bookshelfDock, SIGNAL(moduleAboutTriggered(CSwordModuleInfo*)),
            this, SLOT(moduleAbout(CSwordModuleInfo*)));
}

void BibleTime::initSwordConfigFile() {
// On Windows the sword.conf must be created before the initialization of sword
// It will contain the LocalePath which is used for sword locales
// It also contains a DataPath to the %ALLUSERSPROFILE%\Sword directory
// If this is not done here, the sword locales.d won't be found
#ifdef Q_OS_WIN
    QString configFile = util::directory::getUserHomeSwordDir().filePath("sword.conf");
    QFile file(configFile);
    if (file.exists()) {
        return;
    }
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    QTextStream out(&file);
    out << "\n";
    out << "[Install]\n";
    out << "DataPath="   << util::directory::convertDirSeparators( util::directory::getSharedSwordDir().absolutePath()) << "\n";
    out << "LocalePath=" << util::directory::convertDirSeparators(util::directory::getApplicationSwordDir().absolutePath()) << "\n";
    out << "\n";
    file.close();
#endif

#ifdef Q_OS_MAC
    QString configFile = util::directory::getUserHomeSwordDir().filePath("sword.conf");
    QFile file(configFile);
    if (file.exists()) {
        return;
    }
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    QTextStream out(&file);
    out << "\n";
    out << "[Install]\n";
    out << "DataPath="   << util::directory::convertDirSeparators( util::directory::getUserHomeSwordDir().absolutePath()) << "\n";
    out << "\n";
    file.close();
#endif
}

/** Initializes the backend */
void BibleTime::initBackends() {
    initSwordConfigFile();

    if (!sword::SWMgr::isICU)
        sword::StringMgr::setSystemStringMgr(new BtStringMgr());

    sword::SWLog::getSystemLog()->setLogLevel(btApp->debugMode()
                                              ? sword::SWLog::LOG_DEBUG
                                              : sword::SWLog::LOG_ERROR);

#ifdef Q_OS_MAC
    // set a LocaleMgr with a fixed path to the locales.d of the DMG image on MacOS
    // note: this must be done after setting the BTStringMgr, because this will reset the LocaleMgr
    qDebug() << "Using sword locales dir: " << util::directory::getSwordLocalesDir().absolutePath().toUtf8();
    sword::LocaleMgr::setSystemLocaleMgr(new sword::LocaleMgr(util::directory::getSwordLocalesDir().absolutePath().toUtf8()));
#endif

    /*
      Set book names language if not set. This is a hack. We do this call here,
      because we need to keep the setting displayed in BtLanguageSettingsPage in
      sync with the language of the book names displayed, so that both would
      always use the same setting.
    */
    CDisplaySettingsPage::resetLanguage(); /// \todo refactor this hack


    CSwordBackend *backend = CSwordBackend::createInstance();
    backend->booknameLanguage(btConfig().value<QString>("GUI/booknameLanguage", QLocale::system().name()));

    const CSwordBackend::LoadError errorCode = CSwordBackend::instance()->initModules(CSwordBackend::OtherChange);

    if (errorCode != CSwordBackend::NoError) {
        //show error message that initBackend failed
        /// \todo
//         switch (errorCode) {
//             case CSwordBackend::NoSwordConfig: //mods.d or mods.conf missing
//             {
//                 KStartupLogo::hideSplash();
//                 qDebug() << "case CSwordBackend::NoSwordConfig";
//                 BookshelfManager::CSwordSetupDialog dlg;
//                 dlg.showPart( BookshelfManager::CSwordSetupDialog::Sword );
//                 dlg.exec();
//                 break;
//             }
//
//             case CSwordBackend::NoModules: //no modules installed, but config exists
//             {
//                 KStartupLogo::hideSplash();
//                 qDebug() << "case CSwordBackend::NoModules";
//                 BookshelfManager::CSwordSetupDialog dlg;
//                 dlg.showPart( BookshelfManager::CSwordSetupDialog::Install );
//                 dlg.exec();
//                 break;
//             }
//
//             default: //unknown error
//             {
//                 KStartupLogo::hideSplash();
//                 qDebug() << "unknown error";
//                 BookshelfManager::CSwordSetupDialog dlg;
//                 dlg.showPart( BookshelfManager::CSwordSetupDialog::Sword );
//                 dlg.exec();
//                 break;
//             }
//         }
    }

    // This function will
    // - delete all orphaned indexes (no module present) if autoDeleteOrphanedIndices is true
    // - delete all indices of modules where hasIndex() returns false
    backend->deleteOrphanedIndices();

}

#if BT_DEBUG

QLabel *BibleTime::m_debugWindow = nullptr;
QMutex BibleTime::m_debugWindowLock;

void BibleTime::slotShowDebugWindow(bool show) {
    if (show) {
        QMutexLocker lock(&m_debugWindowLock);
        if (m_debugWindow == nullptr) {
            m_debugWindow = new QLabel(nullptr, Qt::Dialog);
            m_debugWindow->setAttribute(Qt::WA_DeleteOnClose);
            m_debugWindow->setTextFormat(Qt::RichText);
            m_debugWindow->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            m_debugWindow->setWindowTitle(tr("Whats this widget?"));
        }
        m_debugWindow->show();
        connect(m_debugWindow, SIGNAL(destroyed()),
                this,          SLOT(slotDebugWindowClosing()), Qt::DirectConnection);
        QTimer::singleShot(0, this, SLOT(slotDebugTimeout()));
    } else {
        deleteDebugWindow();
    }
}

void BibleTime::deleteDebugWindow() {
    QMutexLocker lock(&m_debugWindowLock);
    if (m_debugWindow != nullptr) {
        m_debugWindow->disconnect(SIGNAL(destroyed()), this, SLOT(slotDebugWindowClosing()));
        delete m_debugWindow;
        m_debugWindow = nullptr;
    }
}

void BibleTime::slotDebugWindowClosing() {
    QMutexLocker lock(&m_debugWindowLock);
    m_debugWindow = nullptr;
    m_debugWidgetAction->setChecked(false);
}

void BibleTime::slotDebugTimeout() {
    QMutexLocker lock(&m_debugWindowLock);
    if (m_debugWindow == nullptr || m_debugWindow->isVisible() == false) return;

    QTimer::singleShot(0, this, SLOT(slotDebugTimeout()));
    QObject *w = QApplication::widgetAt(QCursor::pos());
    if (w != nullptr) {
        QString objectHierarchy;
        do {
            const QMetaObject *m = w->metaObject();
            QString classHierarchy;
            do {
                if (!classHierarchy.isEmpty()) classHierarchy += ": ";
                classHierarchy += m->className();

                m = m->superClass();
            } while (m != nullptr);
            if (!objectHierarchy.isEmpty()) {
                objectHierarchy += "<br/><b>child of:</b> ";
            } else {
                objectHierarchy += "<b>This widget is:</b> ";
            }
            objectHierarchy += classHierarchy;
            w = w->parent();
        } while (w != nullptr);
        m_debugWindow->setText(objectHierarchy);
    } else {
        m_debugWindow->setText("No widget");
    }
    m_debugWindow->resize(m_debugWindow->minimumSizeHint());
}

#endif
