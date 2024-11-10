/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QSortFilterProxyModel>


struct ColumnAndDataPair {
    int column;
    QVariant data;
};

class CellFilteredProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit CellFilteredProxyModel(QObject* parent);

    void addFilter(int columnIndex, const QVariant& data);

    [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex& sourceParent) const override;

private:
    QList<ColumnAndDataPair> filters;
};
