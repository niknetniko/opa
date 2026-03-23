/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "tree_proxy_model.h"

#include "model_utils.h"

TreeProxyModel::TreeProxyModel(QObject* parent) : QAbstractProxyModel(parent) {
}

TreeProxyModel::~TreeProxyModel() {
    clearTree();
}

void TreeProxyModel::setIdColumn(int idColumn) {
    Q_ASSERT(sourceModel());
    Q_ASSERT(0 <= idColumn && idColumn < sourceModel()->columnCount());
    Q_ASSERT(this->parentIdColumn != idColumn);

    beginResetModel();
    this->idColumn = idColumn;
    buildTree();
    endResetModel();
}

void TreeProxyModel::setParentIdColumn(int parentIdColumn) {
    Q_ASSERT(sourceModel());
    Q_ASSERT(0 <= parentIdColumn && parentIdColumn < sourceModel()->columnCount());
    Q_ASSERT(this->idColumn != parentIdColumn);

    beginResetModel();
    this->parentIdColumn = parentIdColumn;
    buildTree();
    endResetModel();
}

void TreeProxyModel::setSourceModel(QAbstractItemModel* newSourceModel) {
    if (sourceModel()) {
        disconnect(sourceModel(), nullptr, this, nullptr);
    }

    QAbstractProxyModel::setSourceModel(newSourceModel);

    if (newSourceModel) {
        connect(newSourceModel, &QAbstractItemModel::modelAboutToBeReset, this, [this] {
            beginResetModel();
            clearTree();
        });
        connect(newSourceModel, &QAbstractItemModel::modelReset, this, [this] {
            buildTree();
            endResetModel();
        });
        connect(newSourceModel, &QAbstractItemModel::rowsInserted, this, [this] {
            beginResetModel();
            buildTree();
            endResetModel();
        });
        connect(newSourceModel, &QAbstractItemModel::rowsRemoved, this, [this] {
            beginResetModel();
            buildTree();
            endResetModel();
        });
        connect(
            newSourceModel,
            &QAbstractItemModel::dataChanged,
            this,
            [this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QList<int>& roles) {
                auto overlaps = [&](int col) { return col >= topLeft.column() && col <= bottomRight.column(); };

                if (overlaps(idColumn) || overlaps(parentIdColumn)) {
                    beginResetModel();
                    buildTree();
                    endResetModel();
                } else {
                    // Forward dataChanged per row, since rows may have different tree parents.
                    for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
                        auto proxyTL = mapFromSource(sourceModel()->index(row, topLeft.column()));
                        auto proxyBR = mapFromSource(sourceModel()->index(row, bottomRight.column()));
                        if (proxyTL.isValid() && proxyBR.isValid()) {
                            Q_EMIT dataChanged(proxyTL, proxyBR, roles);
                        }
                    }
                }
            }
        );
    }
}

void TreeProxyModel::buildTree() {
    clearTree();

    if (!sourceModel() || parentIdColumn < 0) {
        return;
    }

    const int rowCount = sourceModel()->rowCount();

    // First pass: create all nodes and index by ID.
    QHash<QString, TreeNode*> idToNode;
    allNodes.reserve(rowCount);
    for (int row = 0; row < rowCount; ++row) {
        auto node = std::make_unique<TreeNode>();
        node->sourceRow = row;
        auto idVariant = sourceModel()->index(row, idColumn).data();
        if (idVariant.isValid() && !idVariant.isNull()) {
            node->id = idVariant.toString();
            idToNode.insert(node->id, node.get());
        }
        sourceRowToNode.insert(row, node.get());
        allNodes.push_back(std::move(node));
    }

    // Second pass: link children to parents.
    for (const auto& nodePtr: allNodes) {
        auto* node = nodePtr.get();
        auto parentIdVariant = sourceModel()->index(node->sourceRow, parentIdColumn).data();
        if (isInvalid(parentIdVariant)) {
            rootNodes.append(node);
        } else {
            auto parentId = parentIdVariant.toString();
            auto* parentNode = idToNode.value(parentId, nullptr);
            if (parentNode && parentNode != node) {
                node->parent = parentNode;
                parentNode->children.append(node);
            } else {
                rootNodes.append(node);
            }
        }
    }
}

void TreeProxyModel::clearTree() {
    allNodes.clear();
    rootNodes.clear();
    sourceRowToNode.clear();
}

QModelIndex TreeProxyModel::mapFromSource(const QModelIndex& sourceIndex) const {
    if (!sourceIndex.isValid()) {
        return {};
    }

    auto* node = sourceRowToNode.value(sourceIndex.row(), nullptr);
    if (!node) {
        return {};
    }

    const auto& siblings = node->parent ? node->parent->children : rootNodes;
    const int row = siblings.indexOf(node);
    if (row < 0) {
        return {};
    }

    return createIndex(row, sourceIndex.column(), node);
}

QModelIndex TreeProxyModel::mapToSource(const QModelIndex& proxyIndex) const {
    if (!proxyIndex.isValid()) {
        return {};
    }

    auto* node = static_cast<TreeNode*>(proxyIndex.internalPointer());
    if (!node) {
        return {};
    }

    return sourceModel()->index(node->sourceRow, proxyIndex.column());
}

QModelIndex TreeProxyModel::parent(const QModelIndex& child) const {
    if (!child.isValid()) {
        return {};
    }

    auto* node = static_cast<TreeNode*>(child.internalPointer());
    if (!node || !node->parent) {
        return {};
    }

    auto* parentNode = node->parent;
    const auto& grandparentChildren = parentNode->parent ? parentNode->parent->children : rootNodes;
    const int row = grandparentChildren.indexOf(parentNode);

    return createIndex(row, 0, parentNode);
}

bool TreeProxyModel::hasChildren(const QModelIndex& parent) const {
    if (!parent.isValid()) {
        return !rootNodes.isEmpty();
    }

    auto* node = static_cast<TreeNode*>(parent.internalPointer());
    return node && !node->children.isEmpty();
}

QModelIndex TreeProxyModel::sibling(int row, int column, const QModelIndex& idx) const {
    if (row == idx.row() && column == idx.column()) {
        return idx;
    }

    return index(row, column, parent(idx));
}

QModelIndex TreeProxyModel::index(int row, int column, const QModelIndex& parent) const {
    if (parent.isValid() && parent.column() != 0) {
        return {};
    }

    const auto& children = parent.isValid() ? static_cast<TreeNode*>(parent.internalPointer())->children : rootNodes;

    if (row < 0 || row >= children.size()) {
        return {};
    }

    if (!sourceModel() || column < 0 || column >= sourceModel()->columnCount()) {
        return {};
    }

    auto* node = children.at(row);
    return createIndex(row, column, node);
}

Qt::ItemFlags TreeProxyModel::flags(const QModelIndex& index) const {
    auto flags = QAbstractProxyModel::flags(index);
    flags.setFlag(Qt::ItemNeverHasChildren, false);
    return flags;
}

int TreeProxyModel::rowCount(const QModelIndex& parent) const {
    Q_ASSERT(checkIndex(parent));

    if (parentIdColumn < 0 || !sourceModel() || (parent.isValid() && parent.column() != 0)) {
        return 0;
    }

    if (!parent.isValid()) {
        return static_cast<int>(rootNodes.size());
    }

    auto* node = static_cast<TreeNode*>(parent.internalPointer());
    return node ? static_cast<int>(node->children.size()) : 0;
}

int TreeProxyModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    if (!sourceModel()) {
        return 0;
    }
    return sourceModel()->columnCount();
}
