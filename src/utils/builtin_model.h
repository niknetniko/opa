/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <KRearrangeColumnsProxyModel>
#include <QIdentityProxyModel>

/**
 * Model with support for built-in columns.
 *
 * The model will apply a few common things that are needed for working with tables with built-in
 * stuff:
 *
 * - Show an icon on the main column for built-in rows.
 * - Hide the built-in column.
 * - Make built-in rows read-only.
 */
class BuiltinModel : public KRearrangeColumnsProxyModel {
    Q_OBJECT

public:
    explicit BuiltinModel(QObject* parent = nullptr);

    void setColumns(int builtinColumn, int decoratedColumn);

    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;

    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;

    void setSourceModel(QAbstractItemModel* sourceModel) override;

private:
    int decoratedColumn = -1;
    int builtinColumn = -1;

    void syncColumns();
};
