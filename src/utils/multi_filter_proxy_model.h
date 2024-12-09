/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QSortFilterProxyModel>

struct ColumnAndDataPair {
    int column;
    QVariant data;
};

/**
 * A filterable proxy with support for multiple filterable values.
 *
 * For each row, every column will be matched against the expected value.
 * Only rows for which every filter matches will be accepted.
 */
class MultiFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit MultiFilterProxyModel(QObject* parent);

    /**
     * Add a filter for a column.
     *
     * @param columnIndex The column to add a filter for. Adding multiple filters for one row is an error.
     * @param data The data the column should equal.
     */
    void addFilter(int columnIndex, const QVariant& data);

    [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex& sourceParent) const override;

private:
    QList<ColumnAndDataPair> filters;
};
