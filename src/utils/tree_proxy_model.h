/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QIdentityProxyModel>
#include <QVariant>

/**
 * Proxy model that will represent the underlying data as a tree structure.
 *
 * The underlying model has the following requirements:
 *
 * - An "ID" column that uniquely identifies the item.
 * - A "parent ID" column that references the ID column of the parent if the item is a child.
 */
class TreeProxyModel : public QIdentityProxyModel {
    Q_OBJECT

public:
    explicit TreeProxyModel(QObject* parent = nullptr);

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

    [[nodiscard]] QModelIndex mapFromSource(const QModelIndex& sourceIndex) const override;
    [[nodiscard]] QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;

    [[nodiscard]] QModelIndex parent(const QModelIndex& child) const override;

    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex& parent) const override;

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;

private:
    int idColumn = 0;
    int parentIdColumn = -1;

    /**
     * Get the source index of an item with a given ID.
     */
    [[nodiscard]] QModelIndex findSourceItemById(const QVariant& id) const;

    /**
     * Find the row number of an item with a given ID, filtered by the parent ID.
     * This function basically gets all rows with a certain parent ID, and then finds
     * which one is the one with the given ID.
     *
     * @param id The ID of the element to find the row for.
     * @param parentId Optional parent ID. If not provided, it means top-level items.
     */
    [[nodiscard]] int findSourceRowNumberInParent(const QVariant& id, const QVariant& parentId) const;

    /**
     * Find the source row number based on the proxy row number of the item (thus in the parent) and the
     * parent source row number.
     *
     * @param proxyRowInParent The item's row number in the proxy model, beneath the parent.
     * @param parentSourceRow The source row number of the parent or -1 if no parent.
     */
    [[nodiscard]] int findSourceRowNumberByNumberInParent(int proxyRowInParent, int parentSourceRow) const;
};
