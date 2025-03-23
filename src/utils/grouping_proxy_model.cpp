
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "grouping_proxy_model.h"

#include "tree_proxy_model.h"

VirtualParentsModel::VirtualParentsModel(QObject* parent) : KExtraColumnsProxyModel(parent) {
    // TODO: allow setting translations for this?
    // Issue URL: https://github.com/niknetniko/opa/issues/70
    appendColumn(QStringLiteral("group"));
}

void VirtualParentsModel::setGroupedByColumn(int column) {
    Q_ASSERT(0 <= column && column < sourceModel()->columnCount());
    this->groupedByColumn = column;
    calculateGroups();
}

int VirtualParentsModel::rowCount(const QModelIndex& parent) const {
    // This is a flat model.
    if (parent.isValid()) {
        return 0;
    }

    return KExtraColumnsProxyModel::rowCount(parent) + static_cast<int>(groups.size());
}

Qt::ItemFlags VirtualParentsModel::flags(const QModelIndex& index) const {
    if (index.row() >= sourceModel()->rowCount()) {
        return Qt::ItemIsEnabled;
    }
    return KExtraColumnsProxyModel::flags(index);
}

QVariant VirtualParentsModel::extraColumnData(const QModelIndex& parent, int row, int extraColumn, int role) const {
    Q_UNUSED(parent);

    auto sourceRowCount = sourceModel()->rowCount();
    // qDebug() << "extraColumnData: row=" << row << ", extraColumn=" << extraColumn << ", role=" << role;
    if (role != Qt::DisplayRole) {
        return {};
    }

    if (row >= sourceRowCount) {
        Q_ASSERT(extraColumn == 0);
        // These are extra rows for the groups...
        // qDebug() << "extraColumnData: new row, returning data at " << row - sourceRowCount;
        return groups[row - sourceRowCount];
    } else {
        return row;
    }
}

void VirtualParentsModel::calculateGroups() {
    Q_ASSERT(sourceModel());

    beginResetModel();
    groups.clear();

    for (int i = 0; i < sourceModel()->rowCount(); ++i) {
        auto group = sourceModel()->index(i, groupedByColumn).data();
        if (group.isValid() && !groups.contains(group)) {
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
