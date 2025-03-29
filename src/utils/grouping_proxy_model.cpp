
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "grouping_proxy_model.h"

#include "model_utils.h"
#include "tree_proxy_model.h"

VirtualParentsModel::VirtualParentsModel(QObject* parent) : QIdentityProxyModel(parent) {
}

void VirtualParentsModel::setGroupedByColumn(int column) {
    Q_ASSERT(0 <= column && column < sourceModel()->columnCount());
    this->groupedByColumn = column;
    if (sourceModel()) {
        calculateGroups();
    }
}

void VirtualParentsModel::setSourceModel(QAbstractItemModel* sourceModel) {
    QIdentityProxyModel::setSourceModel(sourceModel);
    if (groupedByColumn) {
        calculateGroups();
    }
}

QModelIndex VirtualParentsModel::index(int row, int column, const QModelIndex& parent) const {
    // This is a flat model.
    if (parent.isValid() || row < 0 || column < 0) {
        return {};
    }

    // Only accept indices in our model.
    if (row < sourceModel()->rowCount() && column < sourceModel()->columnCount()) {
        return QIdentityProxyModel::index(row, column);
    }

    return createIndex(row, column);
}

QModelIndex VirtualParentsModel::sibling(int row, int column, const QModelIndex& idx) const {
    if (row == idx.row() && column == idx.column()) {
        return idx;
    }
    return index(row, column, parent(idx));
}

int VirtualParentsModel::rowCount(const QModelIndex& parent) const {
    // This is a flat model.
    if (parent.isValid()) {
        return 0;
    }

    return sourceModel()->rowCount() + static_cast<int>(groups.size());
}

int VirtualParentsModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0; // This is a flat model.
    }

    return sourceModel()->columnCount() + 1;
}

Qt::ItemFlags VirtualParentsModel::flags(const QModelIndex& index) const {
    if (index.row() >= sourceModel()->rowCount() || index.column() >= sourceModel()->columnCount()) {
        return Qt::ItemIsEnabled;
    }

    return QIdentityProxyModel::flags(index);
}

QVariant VirtualParentsModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return {};
    }

    auto sourceRowCount = sourceModel()->rowCount();
    auto sourceColumnCount = sourceModel()->columnCount();

    // These are the "group" cells in the virtual parent rows.
    if (index.row() >= sourceRowCount && index.column() >= sourceColumnCount) {
        Q_ASSERT(index.column() == sourceColumnCount); // We only support one extra column.
        if (role == Qt::DisplayRole) {
            return groups[index.row() - sourceRowCount];
        }
    }
    // These are the virtual rows or the extra column, so these are all empty.
    else if (index.row() >= sourceRowCount || index.column() >= sourceColumnCount) {
        // This should actually be smaller or equal, since we only support one additional column right now.
        Q_ASSERT(index.column() <= sourceColumnCount);
        if (role == Qt::DisplayRole) {
            if (index.column() == sourceColumnCount) {
                return QStringLiteral("Virtual row %1").arg(index.row());
            }
            return QString();
        }
    }
    // This is the normal data.
    else {
        return QIdentityProxyModel::data(index, role);
    }

    return {};
}

QVariant VirtualParentsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && section == sourceModel()->columnCount()) {
        // TODO: let this be configurable.
        // Issue URL: https://github.com/niknetniko/opa/issues/71
        return QStringLiteral("Group");
    }

    return QIdentityProxyModel::headerData(section, orientation, role);
}

void VirtualParentsModel::calculateGroups() {
    Q_ASSERT(sourceModel());

    beginResetModel();
    groups.clear();

    for (int i = 0; i < sourceModel()->rowCount(); ++i) {
        auto group = sourceModel()->index(i, groupedByColumn).data();
        if (!isInvalid(group) && !groups.contains(group)) {
            groups.push_back(group);
        }
    }

    endResetModel();
}

QAbstractProxyModel* createGroupingProxyModel(QAbstractItemModel* original, int groupedByColumn, QObject* parent) {
    auto* virtualRowsModel = new VirtualParentsModel(parent);
    virtualRowsModel->setSourceModel(original);
    virtualRowsModel->setGroupedByColumn(groupedByColumn);

    auto* treeModel = new TreeProxyModel(parent);
    treeModel->setSourceModel(virtualRowsModel);
    treeModel->setIdColumn(virtualRowsModel->columnCount() - 1);
    treeModel->setParentIdColumn(groupedByColumn);

    return treeModel;
}
