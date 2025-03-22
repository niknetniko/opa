/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "tree_proxy_model.h"

TreeProxyModel::TreeProxyModel(QObject* parent) : QIdentityProxyModel(parent) {
}

void TreeProxyModel::setIdColumn(int idColumn) {
    Q_ASSERT(sourceModel());
    Q_ASSERT(0 <= idColumn && idColumn < sourceModel()->columnCount());
    Q_ASSERT(this->parentIdColumn != idColumn);

    this->idColumn = idColumn;
}

void TreeProxyModel::setParentIdColumn(int parentIdColumn) {
    Q_ASSERT(sourceModel());
    Q_ASSERT(0 <= parentIdColumn && parentIdColumn < sourceModel()->columnCount());
    Q_ASSERT(this->idColumn != parentIdColumn);

    this->parentIdColumn = parentIdColumn;
}

QModelIndex TreeProxyModel::mapFromSource(const QModelIndex& sourceIndex) const {
    if (parentIdColumn < 0 || !sourceIndex.isValid()) {
        return {};
    }

    // Check if the source has a parent, in which case we must move it under the parent.
    auto parentId = sourceModel()->index(sourceIndex.row(), parentIdColumn).data();
    auto itemId = sourceModel()->index(sourceIndex.row(), idColumn).data();
    auto rowInParent = findSourceRowNumberInParent(itemId, parentId);

    return createIndex(rowInParent, sourceIndex.column(), sourceIndex.row());
}

QModelIndex TreeProxyModel::mapToSource(const QModelIndex& proxyIndex) const {
    if (!proxyIndex.isValid()) {
        return {};
    }

    int sourceRow = static_cast<int>(proxyIndex.internalId());
    return sourceModel()->index(sourceRow, proxyIndex.column());
}

QModelIndex TreeProxyModel::parent(const QModelIndex& child) const {
    if (parentIdColumn < 0 || !sourceModel() || !child.isValid()) {
        return {};
    }

    auto sourceModelIndex = mapToSource(child);
    auto parentId = sourceModel()->index(sourceModelIndex.row(), parentIdColumn).data();

    if (parentId.isNull()) {
        return {};
    }

    auto sourceParentIndex = findSourceItemById(parentId);
    auto proxyParentIndex = mapFromSource(sourceParentIndex);

    // By convention, the returned column is 0.
    return index(proxyParentIndex.row(), 0, proxyParentIndex.parent());
}

bool TreeProxyModel::hasChildren(const QModelIndex& parent) const {
    return rowCount(parent) > 0;
}

QModelIndex TreeProxyModel::sibling(int row, int column, const QModelIndex& idx) const {
    // Implementation stolen from QAbstractItemModel
    if (row == idx.row() && column == idx.column()) {
        return idx;
    } else {
        return index(row, column, parent(idx));
    }
}

QModelIndex TreeProxyModel::index(int row, int column, const QModelIndex& parent) const {
    if (parent.isValid() && parent.column() != 0) {
        return {};
    }

    int parentSourceRow = parent.isValid() ? static_cast<int>(parent.internalId()) : -1;
    int sourceRow = findSourceRowNumberByNumberInParent(row, parentSourceRow);

    return createIndex(row, column, sourceRow);
}

int TreeProxyModel::rowCount(const QModelIndex& parent) const {
    Q_ASSERT(checkIndex(parent));

    if (parentIdColumn < 0 || !sourceModel() || (parent.isValid() && parent.column() != 0)) {
        return 0;
    }

    QVariant parentId;
    if (parent.isValid()) {
        parentId = index(parent.row(), idColumn, parent.parent()).data();
    }

    int rowsWithParentCount = 0;
    for (int row = 0; row < sourceModel()->rowCount(); ++row) {
        auto rowParentId = sourceModel()->index(row, this->parentIdColumn).data();
        if ((parentId.isValid() && rowParentId == parentId) || (parentId.isNull() && rowParentId.isNull())) {
            rowsWithParentCount++;
        }
    }

    return rowsWithParentCount;
}

int TreeProxyModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return sourceModel()->columnCount();
}

QModelIndex TreeProxyModel::findSourceItemById(const QVariant& id) const {
    Q_ASSERT(sourceModel());

    if (!id.isValid() || id.isNull()) {
        return {};
    }

    auto start = sourceModel()->index(0, this->idColumn);
    auto result = sourceModel()->match(start, Qt::DisplayRole, id, 1, Qt::MatchExactly);

    if (result.isEmpty()) {
        return {};
    }

    return result.first();
}

int TreeProxyModel::findSourceRowNumberInParent(const QVariant& id, const QVariant& parentId) const {
    Q_ASSERT(sourceModel());

    int rowsWithParentCount = 0;
    for (int row = 0; row < sourceModel()->rowCount(); ++row) {
        auto rowParentId = sourceModel()->index(row, this->parentIdColumn).data();
        if ((parentId.isValid() && rowParentId == parentId) || (parentId.isNull() && rowParentId.isNull())) {
            if (sourceModel()->data(sourceModel()->index(row, this->idColumn)) == id) {
                return rowsWithParentCount;
            }
            rowsWithParentCount++;
        }
    }

    throw std::runtime_error("Could not find source row number in parent, are you sure the element has the parent?");
}

int TreeProxyModel::findSourceRowNumberByNumberInParent(int proxyRowInParent, int parentSourceRow) const {
    Q_ASSERT(sourceModel());

    QVariant parentId;
    if (parentSourceRow >= 0) {
        parentId = sourceModel()->index(parentSourceRow, this->idColumn).data();
    }

    int rowsWithParentCount = 0;
    for (int row = 0; row < sourceModel()->rowCount(); ++row) {
        auto rowParentId = sourceModel()->index(row, this->parentIdColumn).data();
        if ((parentId.isValid() && rowParentId == parentId) || (parentId.isNull() && rowParentId.isNull())) {
            if (rowsWithParentCount == proxyRowInParent) {
                return row;
            }
            rowsWithParentCount++;
        }
    }

    throw std::runtime_error("Could not find findSourceRowNumberByNumberInParent");
}
