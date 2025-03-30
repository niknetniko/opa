/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "tree_proxy_model.h"

#include "model_utils.h"

TreeProxyModel::TreeProxyModel(QObject* parent) : QAbstractProxyModel(parent) {
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

    if (isInvalid(parentId)) {
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
    if (row == idx.row() && column == idx.column()) {
        return idx;
    }

    return index(row, column, parent(idx));
}

QModelIndex TreeProxyModel::index(int row, int column, const QModelIndex& parent) const {
    if (!isInvalid(parent) && parent.column() != 0) {
        return {};
    }

    if (sourceModel()->rowCount() <= row || sourceModel()->columnCount() <= column) {
        return {};
    }

    int parentSourceRow = !parent.isValid() ? -1 : static_cast<int>(parent.internalId());
    int sourceRow = findSourceRowNumberByNumberInParent(row, parentSourceRow);

    return createIndex(row, column, sourceRow);
}

Qt::ItemFlags TreeProxyModel::flags(const QModelIndex& index) const {
    auto flags = QAbstractProxyModel::flags(index);
    flags = flags.setFlag(Qt::ItemNeverHasChildren, false);
    return flags;
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
        if (isEqualOrInvalid(parentId, rowParentId)) {
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

    if (isInvalid(id)) {
        return {};
    }

    for (int row = 0; row < sourceModel()->rowCount(); ++row) {
        auto rowIndex = sourceModel()->index(row, this->idColumn);
        if (rowIndex.data() == id) {
            return rowIndex;
        }
    }

    throw std::runtime_error("Did not find source row with ID.");
}

int TreeProxyModel::findSourceRowNumberInParent(const QVariant& id, const QVariant& parentId) const {
    Q_ASSERT(sourceModel());

    int rowsWithParentCount = 0;
    for (int row = 0; row < sourceModel()->rowCount(); ++row) {
        auto rowParentId = sourceModel()->index(row, this->parentIdColumn).data();
        if (isEqualOrInvalid(parentId, rowParentId)) {
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
        if (isEqualOrInvalid(parentId, rowParentId)) {
            if (rowsWithParentCount == proxyRowInParent) {
                return row;
            }
            rowsWithParentCount++;
        }
    }

    throw std::runtime_error("Could not find findSourceRowNumberByNumberInParent");
}

bool TreeProxyModel::isEqualOrInvalid(const QVariant& one, const QVariant& two) const {
    auto isOneInvalid = isInvalid(one);
    auto isTwoInvalid = isInvalid(two);

    if (!isOneInvalid || !isTwoInvalid) {
        return one == two;
    } else {
        // ReSharper disable once CppDFAConstantConditions
        return isOneInvalid && isTwoInvalid;
    }
}
