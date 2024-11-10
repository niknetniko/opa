/*
 * SPDX-FileCopyrightText: 2017 Maxim Paperno (https://github.com/mpaperno/maxLibQt)
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "grouped_items_proxy_model.h"

#include <QQueue>

GroupedItemsProxyModel::GroupedProxyItem::GroupedProxyItem(
    const QModelIndex& srcIndex, const bool isSrcItem, GroupedProxyItem* parent
) :
    m_parentItem(parent),
    m_sourceIndex(srcIndex),
    m_isSourceItem(isSrcItem) {
}

GroupedItemsProxyModel::GroupedProxyItem::GroupedProxyItem(GroupedProxyItem* parent) :
    GroupedProxyItem(QModelIndex(), false, parent) {
}

GroupedItemsProxyModel::GroupedProxyItem::~GroupedProxyItem() {
    clear();
}

void GroupedItemsProxyModel::GroupedProxyItem::clear() {
    qDeleteAll(m_childItems);
    m_childItems.clear();
}

int GroupedItemsProxyModel::GroupedProxyItem::row() const {
    if (parent()) {
        return parent()->childRow(const_cast<GroupedProxyItem*>(this));
    }

    return -1;
}

GroupedItemsProxyModel::GroupedProxyItem*
GroupedItemsProxyModel::GroupedProxyItem::addChild(const QModelIndex& srcIndex, const bool isSrcItem) {
    auto* item = new GroupedProxyItem(srcIndex, isSrcItem, this);
    m_childItems.append(item);
    return item;
}

void GroupedItemsProxyModel::GroupedProxyItem::removeChild(GroupedProxyItem* item) {
    if (item) {
        m_childItems.removeAll(item); // NOLINT(*-err33-c)
        delete item;
    }
}

bool GroupedItemsProxyModel::GroupedProxyItem::setData(const QVariant& data, int role) {
    m_itemData.insert(role, data);
    return true;
}

bool GroupedItemsProxyModel::GroupedProxyItem::setItemData(const QMap<int, QVariant>& roles) {
    bool b = true;
    for (QMap<int, QVariant>::ConstIterator it = roles.begin(); it != roles.end(); ++it) {
        b = b && setData(it.value(), it.key());
    }
    return b;
}


/**
     GroupedItemProxyModel
 */

GroupedItemsProxyModel::GroupedItemsProxyModel(
    QObject* parent, QAbstractItemModel* sourceModel, const QVector<int>& groupColumns
) :
    QIdentityProxyModel(parent),
    m_root(new GroupedProxyItem()) {
    m_root->setData(-1, Qt::EditRole);

    GroupedItemsProxyModel::setGroupMatchRole(Qt::EditRole);
    GroupedItemsProxyModel::setGroupColumnVisible(true);
    GroupedItemsProxyModel::setGroupColumnIsProxy(false);
    GroupedItemsProxyModel::setGroupColumnProxySrc(-1);
    GroupedItemsProxyModel::setGroupRowSelectable(false);
    GroupedItemsProxyModel::setGroupHeaderTitle(tr("Grouping"), tr("This column shows item group information."));

    if (sourceModel) {
        setReloadSuspended(!groupColumns.isEmpty());
        GroupedItemsProxyModel::setSourceModel(sourceModel);
        setReloadSuspended(false);
        if (!groupColumns.isEmpty()) {
            GroupedItemsProxyModel::setGroups(groupColumns);
        } else {
            GroupedItemsProxyModel::reloadSourceModel();
        }
    }
}

GroupedItemsProxyModel::~GroupedItemsProxyModel() {
    delete m_root;
}

void GroupedItemsProxyModel::setSourceModel(QAbstractItemModel* newSourceModel) {
    if (sourceModel()) {
        disconnect(sourceModel(), nullptr, this, nullptr);
    }

    QIdentityProxyModel::setSourceModel(newSourceModel);
    GroupedItemsProxyModel::reloadSourceModel();

    if (sourceModel()) {
        connect(sourceModel(), &QAbstractItemModel::modelReset, this, &GroupedItemsProxyModel::modelResetHandler);
        connect(sourceModel(), &QAbstractItemModel::dataChanged, this, &GroupedItemsProxyModel::dataChangedHandler);
        connect(sourceModel(), &QAbstractItemModel::rowsInserted, this, &GroupedItemsProxyModel::rowsInsertedHandler);
        connect(sourceModel(), &QAbstractItemModel::rowsRemoved, this, &GroupedItemsProxyModel::rowsRemovedHandler);
        connect(sourceModel(), &QAbstractItemModel::rowsMoved, this, &GroupedItemsProxyModel::rowsMovedHandler);
    }
}

QModelIndex GroupedItemsProxyModel::index(int row, int column, const QModelIndex& parent) const {
    if (GroupedProxyItem* childItem = itemForIndex(parent, true)->child(row)) {
        return indexForItem(childItem, column);
    }

    return {};
}

QModelIndex GroupedItemsProxyModel::parent(const QModelIndex& child) const {
    GroupedProxyItem* childItem = itemForIndex(child);
    if (!childItem) {
        return {};
    }

    GroupedProxyItem* parentItem = childItem->parent();
    if (!parentItem || parentItem == m_root) {
        return {};
    }

    return indexForItem(parentItem);
}

int GroupedItemsProxyModel::rowCount(const QModelIndex& parent) const {
    return itemForIndex(parent, true)->rowCount();
}

int GroupedItemsProxyModel::columnCount(const QModelIndex& parent) const {
    return sourceModel()->columnCount(parent) + extraColumns();
}

bool GroupedItemsProxyModel::hasChildren(const QModelIndex& parent) const {
    return rowCount(parent) > 0;
}

QVariant GroupedItemsProxyModel::data(const QModelIndex& index, int role) const {
    GroupedProxyItem* item = itemForIndex(index);
    if (!item) {
        return {};
    }

    if (role == SourceRowNumberRole) {
        return headerData(index.row(), Qt::Vertical, Qt::DisplayRole);
    }

    if (item->isGroupItem() && index.column() == 0) {
        return sourceModel()->data(item->sourceIndex(), role);
    }

    if (isProxyColumn(index)) {
        return sourceModel()->data(sourceIndexForProxy(item), role);
    }

    return sourceModel()->data(mapToSource(index), role);
}

QMap<int, QVariant> GroupedItemsProxyModel::itemData(const QModelIndex& index) const {
    GroupedProxyItem* item = itemForIndex(index);
    if (!item) {
        return {};
    }

    if (item->isGroupItem() && index.column() == 0) {
        return sourceModel()->itemData(item->sourceIndex());
    }

    if (isProxyColumn(index)) {
        return sourceModel()->itemData(sourceIndexForProxy(item));
    }

    return sourceModel()->itemData(mapToSource(index));
}

QVariant GroupedItemsProxyModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal) {
        if (extraColumns() && section == 0 && (role == Qt::DisplayRole || role == Qt::ToolTipRole)) {
            return m_root->data(role);
        }

        return sourceModel()->headerData(section - extraColumns(), orientation, role);
    }
    if (orientation == Qt::Vertical) {
        if (GroupedProxyItem* item = itemForRow(section); item && item->isSourceItem()) {
            return sourceModel()->headerData(section, orientation, role);
        }
    }
    return {};
}

Qt::ItemFlags GroupedItemsProxyModel::flags(const QModelIndex& index) const {
    GroupedProxyItem* item = itemForIndex(index);
    if (!item) {
        return Qt::NoItemFlags;
    }

    if (item->isGroupItem()) {
        if (groupRowSelectable()) {
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        }
        return Qt::ItemIsEnabled;
    }

    if (isProxyColumn(index)) {
        return sourceModel()->flags(sourceIndexForProxy(item)) & ~Qt::ItemIsEditable;
    }

    return sourceModel()->flags(mapToSource(index));
}

QSize GroupedItemsProxyModel::span(const QModelIndex& index) const {
    GroupedProxyItem* item = itemForIndex(index);
    if (!item) {
        return {};
    }

    if (item->isSourceItem()) {
        return sourceModel()->span(mapToSource(index));
    }

    return {columnCount(), 1};
}

QModelIndex GroupedItemsProxyModel::mapToSource(const QModelIndex& proxyIndex) const {
    if (GroupedProxyItem* item = itemForIndex(proxyIndex, false); item && item->isSourceItem()) {
        // qDebug() << proxyIndex << item->sourceIndex << sourceModel()->index(item->sourceRow(),
        // proxyIndex.column() - extraColumns());
        return sourceModel()->index(item->sourceIndex().row(), proxyIndex.column() - extraColumns());
    }

    return {};
}

QModelIndex GroupedItemsProxyModel::mapFromSource(const QModelIndex& sourceIndex) const {
    if (GroupedProxyItem* item = nullptr;
        m_sourceMap.contains(sourceIndex.row()) &&
        ((item = m_sourceMap.value(sourceIndex.row())))) { // NOLINT(*-assignment-in-if-condition)
        // qDebug() << sourceIndex << indexForItem(item, sourceIndex.column() + extraColumns());
        return indexForItem(item, sourceIndex.column() + extraColumns());
    }

    return {};
}

void GroupedItemsProxyModel::addGroups(const QVector<int>& columns) {
    if (columns.isEmpty()) {
        return;
    }
    const bool prev = setReloadSuspended(true);
    for (const int i: columns) {
        GroupedItemsProxyModel::insertGroup(static_cast<int>(m_groups.size()), i);
    }
    setReloadSuspended(prev);
    GroupedItemsProxyModel::reloadSourceModel();
}

void GroupedItemsProxyModel::setGroups(const QVector<int>& columns) {
    if (columns.isEmpty()) {
        GroupedItemsProxyModel::clearGroups();
        return;
    }
    const bool prev = setReloadSuspended(true);
    GroupedItemsProxyModel::clearGroups();
    GroupedItemsProxyModel::addGroups(columns);
    setReloadSuspended(prev);
    GroupedItemsProxyModel::reloadSourceModel();
}

void GroupedItemsProxyModel::insertGroup(int index, int column) {
    index = static_cast<int>(qBound(0, index, m_groups.size()));
    if (m_groups.indexOf(column) == -1) {
        m_groups.insert(index, column);
        GroupedItemsProxyModel::reloadSourceModel();
    }
}

void GroupedItemsProxyModel::removeGroup(int column) {
    if (m_groups.removeAll(column)) {
        GroupedItemsProxyModel::reloadSourceModel();
    }
}

void GroupedItemsProxyModel::clearGroups() {
    if (m_groups.isEmpty()) {
        return;
    }
    const bool prev = setReloadSuspended(true);
    for (const int i: m_groups) {
        GroupedItemsProxyModel::removeGroup(i);
    }
    setReloadSuspended(prev);
    GroupedItemsProxyModel::reloadSourceModel();
}

void GroupedItemsProxyModel::setGroupMatchRole(int role) {
    if (m_groupMatchRole != role) {
        m_groupMatchRole = role;
        if (sourceModel() && !m_groups.isEmpty()) {
            GroupedItemsProxyModel::reloadSourceModel();
        }
    }
}

void GroupedItemsProxyModel::setGroupHeaderTitle(const QString& title, const QString& tooltip) {
    m_root->setData(title, Qt::DisplayRole);
    if (!tooltip.isEmpty()) {
        m_root->setData(tooltip, Qt::ToolTipRole);
    } else {
        m_root->setData(title, Qt::ToolTipRole);
    }
    if (GroupedItemsProxyModel::extraColumns()) {
        Q_EMIT headerDataChanged(Qt::Horizontal, 0, 0);
    }
}


//
// protected methods
//

QModelIndex GroupedItemsProxyModel::indexForItem(GroupedProxyItem* item, const int col) const {
    if (!item || item->row() < 0 || col < 0) {
        return {};
    }

    return createIndex(item->row(), col, item);
}

GroupedItemsProxyModel::GroupedProxyItem*
GroupedItemsProxyModel::itemForIndex(const QModelIndex& index, const bool rootDefault) const {
    if (GroupedProxyItem* item = nullptr;
        index.isValid() &&
        ((item = static_cast<GroupedProxyItem*>(index.internalPointer())))) { // NOLINT(*-assignment-in-if-condition)
        return item;
    }
    if (rootDefault) {
        return m_root;
    }
    return nullptr;
}

GroupedItemsProxyModel::GroupedProxyItem* GroupedItemsProxyModel::findGroupItem( // NOLINT(*-no-recursion)
    const int group,
    const QVariant& value,
    GroupedProxyItem* parent
) const {
    if (!parent) {
        parent = m_root;
    }
    for (GroupedProxyItem* item: parent->children()) {
        if (!item) {
            continue;
        }
        if (!item->isSourceItem() && item->data(Qt::UserRole).toInt() == group && item->data(Qt::EditRole) == value) {
            return item;
        }
        if (item->rowCount() && ((item = findGroupItem(group, value, item)))) { // NOLINT(*-assignment-in-if-condition)
            return item;
        }
    }
    return nullptr;
}

QModelIndex GroupedItemsProxyModel::sourceIndexForProxy(GroupedProxyItem* item) const {
    QModelIndex srcIdx = item->sourceIndex();
    if (groupColumnProxySrc() > -1) {
        srcIdx = sourceModel()->index(srcIdx.row(), groupColumnProxySrc(), srcIdx.parent());
    }
    return srcIdx;
}

// NOLINTNEXTLINE(*-no-recursion)
int GroupedItemsProxyModel::totalRowCount(const GroupedProxyItem* parent) const {
    if (!parent) {
        parent = m_root;
    }
    int count = 0;
    for (GroupedProxyItem* item: parent->children()) {
        ++count;
        if (item->isGroupItem()) {
            count += totalRowCount(item);
        }
    }
    return count;
}

GroupedItemsProxyModel::GroupedProxyItem* GroupedItemsProxyModel::itemForRow(int row, GroupedProxyItem* parent) const {
    GroupedProxyItem* ret = nullptr;
    if (!parent) {
        parent = m_root;
    }
    QQueue<GroupedProxyItem*> q;
    q.enqueue(parent);
    int count = 0;
    while (!q.isEmpty()) {
        for (GroupedProxyItem* p = q.dequeue(); GroupedProxyItem * item: p->children()) {
            if (count++ == row) {
                return item;
            }
            if (item->rowCount()) {
                q.enqueue(item);
            }
        }
    }
    return ret;
}

GroupedItemsProxyModel::GroupedProxyItem* GroupedItemsProxyModel::placeSourceRow(const int row) {
    GroupedProxyItem* grpParent = m_root;
    QModelIndex sourceIndex = sourceModel()->index(row, 0);
    for (const int col: m_groups) {
        sourceIndex = sourceModel()->index(row, col);
        const auto val = sourceIndex.data(groupMatchRole());
        GroupedProxyItem* grpItem = nullptr;
        // NOLINTNEXTLINE(*-assignment-in-if-condition)
        if (!((grpItem = findGroupItem(col, val, grpParent)))) {
            beginInsertRows(indexForItem(grpParent), grpParent->rowCount(), grpParent->rowCount());
            grpItem = grpParent->addChild(sourceIndex, false);
            grpItem->setData(col, Qt::UserRole);
            grpItem->setData(val, Qt::EditRole);
            endInsertRows();
        }
        grpParent = grpItem;
    }
    beginInsertRows(indexForItem(grpParent), grpParent->rowCount(), grpParent->rowCount());
    GroupedProxyItem* newItem = grpParent->addChild(sourceIndex);
    m_sourceMap.insert(row, newItem);
    endInsertRows();

    return newItem;
}

void GroupedItemsProxyModel::removeItem(GroupedProxyItem* item) {
    if (!item || !item->parent()) {
        return;
    }

    GroupedProxyItem* parent = item->parent();
    beginRemoveRows(indexForItem(parent), item->row(), item->row());
    if (item->isSourceItem()) {
        m_sourceMap.remove(item->sourceIndex().row()); // NOLINT(*-err33-c)
    }
    parent->removeChild(item);
    endRemoveRows();
}

// NOLINTNEXTLINE(*-no-recursion)
void GroupedItemsProxyModel::removeUnusedGroups(GroupedProxyItem* parent) {
    if (!parent) {
        parent = m_root;
    }
    for (GroupedProxyItem* item: parent->children()) {
        if (item->isSourceItem()) {
            continue;
        }
        if (item->rowCount()) {
            removeUnusedGroups(item);
        }
        if (!item->rowCount()) {
            removeItem(item);
        }
    }
}

void GroupedItemsProxyModel::reloadSourceModel() {
    if (reloadSuspended()) {
        return;
    }
    beginResetModel();
    m_root->clear();
    m_sourceMap.clear();
    for (int row = 0, e = sourceModel()->rowCount(); row < e; ++row) {
        placeSourceRow(row);
    }
    endResetModel();
}

void GroupedItemsProxyModel::modelResetHandler() {
    reloadSourceModel();
}

void GroupedItemsProxyModel::dataChangedHandler(
    const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles
) {
    // qDebug() << topLeft << bottomRight << roles;
    if (!topLeft.isValid() || !bottomRight.isValid() || m_groups.isEmpty() ||
        (!roles.isEmpty() && !roles.contains(groupMatchRole()))) {
        return;
    }

    const QModelIndex& srcParent = topLeft.parent();
    const int endRow = qMin(bottomRight.row(), sourceModel()->rowCount());
    const int startCol = qMax(topLeft.column(), 0);
    const int endCol = qMin(bottomRight.column(), sourceModel()->columnCount() - 1);
    bool modified = false;
    for (int row = topLeft.row(); row <= endRow; ++row) {
        for (const int col: m_groups) {
            if (startCol < col || endCol > col) {
                continue;
            } // not a column we care about

            const QModelIndex& srcIdx = sourceModel()->index(row, col, srcParent);
            GroupedProxyItem* currItem = itemForIndex(mapFromSource(srcIdx), false);
            if (!currItem) {
                // source model is out of sync (shouldn't happen)
                reloadSourceModel();
                return;
            }
            GroupedProxyItem* currParent = currItem->parent();
            if (GroupedProxyItem* newParent = findGroupItem(col, srcIdx.data(groupMatchRole()));
                newParent && currParent == newParent) {
                continue;
            } // the parent group hasn't changed

            // grouping value has changed
            removeItem(currItem);
            placeSourceRow(row);
            modified = true;
            break;
        }
    }
    if (modified) {
        removeUnusedGroups();
    }
}

void GroupedItemsProxyModel::rowsInsertedHandler(const QModelIndex& parent, int first, int last) {
    Q_UNUSED(parent)
    for (int row = first; row <= last; ++row) {
        placeSourceRow(row);
    }
}

void GroupedItemsProxyModel::rowsRemovedHandler([[maybe_unused]] const QModelIndex& parent, int first, int last) {
    for (int row = first; row <= last; ++row) {
        GroupedProxyItem* currItem = m_sourceMap.value(row, nullptr);
        if (!currItem || !currItem->parent()) {
            // source model is out of sync (shouldn't happen)
            reloadSourceModel();
            return;
        }
        removeItem(currItem);
    }
    removeUnusedGroups();
}

void GroupedItemsProxyModel::rowsMovedHandler(
    const QModelIndex& parent, int start, int end, const QModelIndex& destination, int row
) {
    rowsRemovedHandler(parent, start, end);
    rowsInsertedHandler(destination.parent(), row, row + (end - start));
}
