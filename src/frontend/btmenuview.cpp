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

#include "frontend/btmenuview.h"

#include <QActionGroup>


BtMenuView::BtMenuView(QWidget *parent)
    : QMenu(parent), m_model(nullptr), m_parentIndex(QModelIndex()), m_actions(nullptr)
{
    connect(this, SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShow()));
    connect(this, SIGNAL(triggered(QAction*)),
            this, SLOT(slotActionTriggered(QAction*)));
}

BtMenuView::~BtMenuView() {
    delete m_actions;
}

void BtMenuView::setModel(QAbstractItemModel *model) {
    m_model = model;
    delete m_actions;
    m_actions = nullptr;
    m_indexMap.clear();
    m_parentIndex = QModelIndex();
}

void BtMenuView::setParentIndex(const QModelIndex &parentIndex) {
    if (parentIndex.isValid() && parentIndex.model() != m_model) return;
    m_parentIndex = parentIndex;
}

void BtMenuView::preBuildMenu() {
    // Intentionally empty. Reimplement in subclass if needed.
}

void BtMenuView::postBuildMenu() {
    // Intentionally empty. Reimplement in subclass if needed.
}

QAction *BtMenuView::newAction(QMenu *parentMenu, const QModelIndex &itemIndex) {
    QVariant displayData(m_model->data(itemIndex, Qt::DisplayRole));
    QVariant iconData(m_model->data(itemIndex, Qt::DecorationRole));
    QVariant toolTipData(m_model->data(itemIndex, Qt::ToolTipRole));
    QVariant statusTipData(m_model->data(itemIndex, Qt::StatusTipRole));
    QVariant whatsThisData(m_model->data(itemIndex, Qt::WhatsThisRole));

    QAction *childAction = new QAction(parentMenu);

    // Set text:
    if (displayData.canConvert(QVariant::String)) {
        childAction->setText(displayData.toString());
    }

    // Set icon:
    if (iconData.canConvert(QVariant::Icon)) {
        childAction->setIcon(iconData.value<QIcon>());
    }

    // Set tooltip:
    if (toolTipData.canConvert(QVariant::String)) {
        childAction->setToolTip(toolTipData.toString());
    }

    // Set status tip:
    if (statusTipData.canConvert(QVariant::String)) {
        childAction->setStatusTip(statusTipData.toString());
    }

    // Set whatsthis:
    if (whatsThisData.canConvert(QVariant::String)) {
        childAction->setWhatsThis(whatsThisData.toString());
    }

    // Set checkable:
    if (m_model->flags(itemIndex).testFlag(Qt::ItemIsUserCheckable)) {
        childAction->setCheckable(true);
    }

    // Set checked:
    QVariant checkData(m_model->data(itemIndex, Qt::CheckStateRole));
    bool ok;
    Qt::CheckState const state =
            static_cast<Qt::CheckState>(checkData.toInt(&ok));
    if (ok)
        childAction->setChecked(state == Qt::Checked);

    return childAction;
}

QMenu *BtMenuView::newMenu(QMenu *parentMenu, const QModelIndex &itemIndex) {
    QVariant displayData(m_model->data(itemIndex, Qt::DisplayRole));
    QVariant iconData(m_model->data(itemIndex, Qt::DecorationRole));
    QVariant toolTipData(m_model->data(itemIndex, Qt::ToolTipRole));
    QVariant statusTipData(m_model->data(itemIndex, Qt::StatusTipRole));
    QVariant whatsThisData(m_model->data(itemIndex, Qt::WhatsThisRole));

    QMenu *childMenu = new QMenu(parentMenu);

    // Set text:
    if (displayData.canConvert(QVariant::String)) {
        childMenu->setTitle(displayData.toString());
    }

    // Set icon:
    if (iconData.canConvert(QVariant::Icon)) {
        childMenu->setIcon(iconData.value<QIcon>());
    }

    // Set tooltip:
    if (toolTipData.canConvert(QVariant::String)) {
        childMenu->setToolTip(toolTipData.toString());
    }

    // Set status tip:
    if (statusTipData.canConvert(QVariant::String)) {
        childMenu->setStatusTip(statusTipData.toString());
    }

    // Set whatsthis:
    if (whatsThisData.canConvert(QVariant::String)) {
        childMenu->setWhatsThis(whatsThisData.toString());
    }

    return childMenu;
}

void BtMenuView::buildMenu(QMenu *parentMenu, const QModelIndex &parentIndex) {
    Q_ASSERT(m_model != nullptr);
    Q_ASSERT(m_actions != nullptr);

    int children = m_model->rowCount(parentIndex);
    for (int i = 0; i < children; i++) {
        QModelIndex childIndex(m_model->index(i, 0, parentIndex));

        if (m_model->rowCount(childIndex) > 0) {
            QMenu *childMenu = newMenu(parentMenu, childIndex);

            if (childMenu != nullptr) {
                // Add the child menu and populate it:
                parentMenu->addMenu(childMenu);
                buildMenu(childMenu, childIndex);
            }
        } else {
            QAction *childAction = newAction(parentMenu, childIndex);

            if (childAction != nullptr) {
                // Map index
                m_indexMap.insert(childAction, childIndex);

                // Add action to action group:
                childAction->setActionGroup(m_actions);

                // Add action to menu:
                parentMenu->addAction(childAction);
            }
        }
    }
}

void BtMenuView::slotAboutToShow() {
    // The signal "aboutToHide" comes before the signal "triggered" and
    // leads to executing a deleted action and a crash. It is much safer
    // to remove the menus here.
    removeMenus();
    delete m_actions;
    m_actions = nullptr;
    m_indexMap.clear();

    preBuildMenu();

    if (m_model != nullptr) {
        m_actions = new QActionGroup(this);

        buildMenu(this, m_parentIndex);
    }
    postBuildMenu();
}

void BtMenuView::removeMenus() {
    // QMenu::clear() is documented only to delete direct child actions:
    clear();

    // Delete submenus also:
    Q_FOREACH (QObject * const child, children())
        delete qobject_cast<QMenu *>(child);
}

void BtMenuView::slotActionTriggered(QAction *action) {
    if (!m_indexMap.contains(action)) return;
    QPersistentModelIndex itemIndex(m_indexMap.value(action));
    if (itemIndex.isValid()) {
        emit triggered(itemIndex);
    }
}
