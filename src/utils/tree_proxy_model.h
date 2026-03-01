/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QAbstractProxyModel>
#include <QHash>
#include <QList>
#include <QVariant>

#include <memory>
#include <vector>

/**
 * Proxy model that will represent the underlying data as a tree structure.
 *
 * The underlying model has the following requirements:
 *
 * - An "ID" column that uniquely identifies the item.
 * - A "parent ID" column that references the ID column of the parent if the item is a child.
 *
 * The tree order mirrors the source model's row order.
 */
class TreeProxyModel : public QAbstractProxyModel {
    Q_OBJECT

public:
    explicit TreeProxyModel(QObject* parent = nullptr);
    ~TreeProxyModel() override;

    /**
     * Set the column to be used as the ID column.
     * By default, the first column of the source model is used.
     * Call this after you have set the source model.
     */
    void setIdColumn(int idColumn);

    /**
     * Set the column to be used as the parent ID column.
     * Call this after you have set the source model.
     */
    void setParentIdColumn(int parentIdColumn);

    void setSourceModel(QAbstractItemModel* sourceModel) override;

    [[nodiscard]] QModelIndex mapFromSource(const QModelIndex& sourceIndex) const override;
    [[nodiscard]] QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;

    [[nodiscard]] QModelIndex parent(const QModelIndex& child) const override;
    [[nodiscard]] bool hasChildren(const QModelIndex& parent) const override;
    [[nodiscard]] QModelIndex sibling(int row, int column, const QModelIndex& idx) const override;

    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;

private:
    struct TreeNode {
        int sourceRow;
        QString id;
        TreeNode* parent = nullptr;
        QList<TreeNode*> children;
    };

    int idColumn = 0;
    int parentIdColumn = -1;

    QList<TreeNode*> rootNodes;
    std::vector<std::unique_ptr<TreeNode>> allNodes;
    QHash<int, TreeNode*> sourceRowToNode;

    void buildTree();
    void clearTree();
};