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

bool TreeProxyModel::hasChildren(const QModelIndex& parent) const {
    if (!sourceModel()) {
        return false;
    }

    if (!parent.isValid()) {
        // Root level: Check if there are any items without a parent
        return rowCount(parent) > 0;
    }

    if (parent.column() != 0) {
        return false; // Only the first column has children, by convention.
    }

    QVariant parentId = data(parent, parentIdColumn);
    if (!parentId.isValid()) {
        return false;
    }

    for (int row = 0; row < sourceModel()->rowCount(); ++row) {
        QModelIndex sourceParentIdIndex = sourceModel()->index(row, parentIdColumn);
        if (sourceModel()->data(sourceParentIdIndex) == parentId) {
            return true;
        }
    }

    return false;
}

int TreeProxyModel::rowCount(const QModelIndex& parent) const {
    if (!sourceModel()) {
        return 0;
    }

    if (parent.isValid() && parent.column() != 0) {
        return 0; // Only the first column can have children.
    }

    int count = 0;
    if (parent.isValid()) {
        // Count children of the given parent.
        QVariant parentId = data(parent, parentIdColumn);
        if (!parentId.isValid()) {
            return 0;
        }

        for (int row = 0; row < sourceModel()->rowCount(); ++row) {
            if (sourceModel()->index(row, parentIdColumn).data() == parentId) {
                count++;
            }
        }
    } else {
        // Count top-level items (no parent ID in the source model).
        for (int row = 0; row < sourceModel()->rowCount(); ++row) {
            if (sourceModel()->index(row, parentIdColumn).data().isNull()) {
                count++;
            }
        }
    }

    return count;
}

QModelIndex TreeProxyModel::parent(const QModelIndex& child) const {
    if (!sourceModel() || !child.isValid()) {
        return {};
    }

    auto sourceChild = mapToSource(child);
    auto parentId = sourceModel()->index(sourceChild.row(), parentIdColumn).data();
    if (parentId.isNull()) {
        return {};
    }

    // Find the *proxy* index of the parent.
    for (int row = 0; row < QIdentityProxyModel::rowCount({}); ++row) {
        QModelIndex proxyParentCandidate = QIdentityProxyModel::index(row, 0, {});
        QVariant candidateId = data(proxyParentCandidate, parentIdColumn);
        if (candidateId == parentId) {
            return proxyParentCandidate;
        }

        // Now check the second level.
        for (int childRow = 0; childRow < QIdentityProxyModel::rowCount(proxyParentCandidate); ++childRow) {
            QModelIndex secondLevelProxyParentCandidate = QIdentityProxyModel::index(childRow, 0, proxyParentCandidate);
            QVariant secondLevelCandidateId = data(secondLevelProxyParentCandidate, parentIdColumn);
            if (secondLevelCandidateId == parentId) {
                return secondLevelProxyParentCandidate;
            }
        }
    }

    return {};
}

QModelIndex TreeProxyModel::index(int row, int column, const QModelIndex& parent) const {
    if (!sourceModel() || row < 0 || column < 0) {
        return {};
    }
    if (parent.isValid() && parent.column() != 0) {
        return {};
    }

    QVariant parentId;
    if (parent.isValid()) {
        parentId = data(parent, parentIdColumn);
    }

    int proxyRowCounter = 0;
    for (int sourceRow = 0; sourceRow < sourceModel()->rowCount(); ++sourceRow) {
        QModelIndex sourceParentIdIndex = sourceModel()->index(sourceRow, parentIdColumn);
        // Match against parentId (if parent is valid) or null (if parent is invalid/root)
        if ((parent.isValid() && sourceModel()->data(sourceParentIdIndex) == parentId) ||
            (!parent.isValid() && sourceModel()->data(sourceParentIdIndex).isNull())) {

            if (proxyRowCounter == row) {
                // Found the correct source row.
                QModelIndex sourceIndex = sourceModel()->index(sourceRow, column);
                // Use mapFromSource for consistent index creation.
                QModelIndex proxyIndex = QIdentityProxyModel::mapFromSource(sourceIndex);
                return proxyIndex;
            }
            proxyRowCounter++;
        }
    }

    return {};
}
bool TreeProxyModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    return QIdentityProxyModel::setData(index, value, role);
}
