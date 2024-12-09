/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "multi_filter_proxy_model.h"

MultiFilterProxyModel::MultiFilterProxyModel(QObject* parent) : QSortFilterProxyModel(parent) {
}

void MultiFilterProxyModel::addFilter(int columnIndex, const QVariant& data) {
    Q_ASSERT(std::none_of(filters.constBegin(), filters.constEnd(), [columnIndex](const ColumnAndDataPair& it) {
        return it.column == columnIndex;
    }));
    this->filters.append({.column = columnIndex, .data = data});
}

bool MultiFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& sourceParent) const {
    return std::all_of(filters.constBegin(), filters.constEnd(), [&](const ColumnAndDataPair& it) {
        auto index = sourceModel()->index(source_row, it.column, sourceParent);
        return index.data() == it.data;
    });
}
