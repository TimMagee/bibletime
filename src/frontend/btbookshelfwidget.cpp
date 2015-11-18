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

#include "frontend/btbookshelfwidget.h"

#include <QActionGroup>
#include <QApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QToolButton>
#include <QVBoxLayout>
#include "backend/bookshelfmodel/btbookshelffiltermodel.h"
#include "bibletimeapp.h"
#include "frontend/bookshelfmanager/installpage/btinstallmodulechooserdialogmodel.h"
#include "frontend/bookshelfmanager/removepage/btremovepagetreemodel.h"
#include "frontend/btbookshelfdockwidget.h"
#include "frontend/btbookshelfgroupingmenu.h"
#include "frontend/btbookshelfview.h"
#include "util/cresmgr.h"
#include "util/directory.h"


BtBookshelfWidget::BtBookshelfWidget(QWidget *parent, Qt::WindowFlags flags)
        : QWidget(parent, flags)
        , m_sourceModel(nullptr)
        , m_treeModel(nullptr)
        , m_leftCornerWidget(nullptr)
        , m_rightCornerWidget(nullptr)
{
    // Setup post-filter:
    m_postFilterModel = new BtBookshelfFilterModel(this);

    // Init widgets and such:
    initActions();
    initMenus();
    initWidgets();

    // Connect treeview to model:
    m_treeView->setModel(m_postFilterModel);

    retranslateUi();

    connect(m_nameFilterEdit,  SIGNAL(textEdited(QString)),
            m_postFilterModel, SLOT(setNameFilterFixedString(QString)));
    connect(m_treeView, SIGNAL(contextMenuActivated(QPoint)),
            this,       SLOT(slotShowContextMenu(QPoint)));
    connect(m_treeView, SIGNAL(moduleContextMenuActivated(CSwordModuleInfo*, QPoint)),
            this,       SLOT(slotShowItemContextMenu(CSwordModuleInfo*, QPoint)));
}

void BtBookshelfWidget::setSourceModel(QAbstractItemModel *model) {
    Q_ASSERT(model != nullptr);
    m_sourceModel = model;
    if (m_treeModel != nullptr) {
        m_treeModel->setSourceModel(model);
    }
}

void BtBookshelfWidget::setTreeModel(BtBookshelfTreeModel *model) {
    Q_ASSERT(model != nullptr);
    Q_ASSERT(m_treeModel == nullptr);
    m_treeModel = model;
    if (m_sourceModel != nullptr) {
        model->setSourceModel(m_sourceModel);
    }
    m_postFilterModel->setSourceModel(model);
}

void BtBookshelfWidget::setLeftCornerWidget(QWidget *w) {
    delete m_leftCornerWidget;
    w->setParent(this);
    m_toolBar->insertWidget(0, w, 0);
}

void BtBookshelfWidget::setRightCornerWidget(QWidget *w) {
    delete m_rightCornerWidget;
    w->setParent(this);
    m_toolBar->insertWidget(m_toolBar->count(), w, 0);
}

void BtBookshelfWidget::initActions() {
    m_showHideAction = new QAction(this);
    m_showHideAction->setIcon(CResMgr::mainIndex::showHide::icon());
    m_showHideAction->setCheckable(true);
    connect(m_showHideAction, SIGNAL(toggled(bool)),
            m_postFilterModel, SLOT(setShowHidden(bool)));
}

void BtBookshelfWidget::initMenus() {
    // Grouping menu:
    m_groupingMenu = new BtBookshelfGroupingMenu(this);
    connect(m_groupingMenu, SIGNAL(signalGroupingOrderChanged(BtBookshelfTreeModel::Grouping)),
            this,           SLOT(slotGroupingActionTriggered(BtBookshelfTreeModel::Grouping)));

    // Context menu
    m_contextMenu = new QMenu(this);
    m_contextMenu->addMenu(m_groupingMenu);
    m_contextMenu->addAction(m_showHideAction);

    // Item context menu
    m_itemContextMenu = m_contextMenu;
}

void BtBookshelfWidget::initWidgets() {
    QVBoxLayout *layout(new QVBoxLayout);
    layout->setContentsMargins(0, 8, 0, 0);
    m_toolBar = new QHBoxLayout;
    // Add a small margin between the edge of the window and the label (looks better)
    m_toolBar->setContentsMargins(3, 0, 0, 0);
    m_nameFilterLabel = new QLabel(this);
    m_toolBar->addWidget(m_nameFilterLabel);

    m_nameFilterEdit = new QLineEdit(this);
    m_nameFilterEdit->installEventFilter(this);
    m_nameFilterLabel->setBuddy(m_nameFilterEdit);
    m_toolBar->addWidget(m_nameFilterEdit);

    m_groupingButton = new QToolButton(this);
    m_groupingButton->setPopupMode(QToolButton::InstantPopup);
    m_groupingButton->setMenu(m_groupingMenu);
    m_groupingButton->setIcon(m_groupingMenu->icon());
    m_groupingButton->setAutoRaise(true);
    m_toolBar->addWidget(m_groupingButton);

    m_showHideButton = new QToolButton(this);
    m_showHideButton->setDefaultAction(m_showHideAction);
    m_showHideButton->setAutoRaise(true);
    m_toolBar->addWidget(m_showHideButton);
    layout->addLayout(m_toolBar);

    m_treeView = new BtBookshelfView(this);
    layout->addWidget(m_treeView);
    setLayout(layout);
}

void BtBookshelfWidget::retranslateUi() {
    m_nameFilterLabel->setText(tr("Fi&lter:"));
    m_groupingButton->setText(tr("Grouping"));
    m_groupingButton->setToolTip(tr("Change the grouping of items in the bookshelf."));
    m_groupingMenu->setTitle(tr("Grouping"));
    m_showHideAction->setText(tr("Show/hide works"));
}

bool BtBookshelfWidget::eventFilter(QObject *object, QEvent *event) {
    Q_ASSERT(object == m_nameFilterEdit);
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *e = static_cast<QKeyEvent*>(event);
        switch (e->key()) {
            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_Enter:
            case Qt::Key_Return:
                QApplication::sendEvent(m_treeView, event);
                return true;
            default:
                break;
        }
    }
    return false;
}

void BtBookshelfWidget::slotGroupingActionTriggered(const BtBookshelfTreeModel::Grouping &grouping) {
    m_treeModel->setGroupingOrder(grouping);
    m_treeView->setRootIsDecorated(!grouping.isEmpty());
}

void BtBookshelfWidget::slotShowContextMenu(const QPoint &pos) {
    m_contextMenu->popup(pos);
}

void BtBookshelfWidget::slotShowItemContextMenu(CSwordModuleInfo *module, const QPoint &pos)
{
    if (m_itemContextMenu != nullptr) {
        m_itemContextMenu->setProperty("BtModule",
                                       qVariantFromValue(
                                           static_cast<void *>(module)));
        m_itemContextMenu->popup(pos);
    } else {
        m_itemContextMenu = m_contextMenu;
        slotShowItemContextMenu(module, pos);
    }
}
