/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QIdentityProxyModel>

/**
 * Proxy model that will represent the underlying data as a tree structure.
 *
 * The underlying model has the following requirements:
 *
 * - An "ID" column that uniquely identifies the item.
 * - A "parent ID" column that references the ID column of the parent if the item is a child.
 *
 * TODO: The model supports editing.
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

    bool hasChildren(const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;

private:
    int idColumn = 0;
    int parentIdColumn = -1;
};
