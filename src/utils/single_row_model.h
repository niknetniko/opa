#pragma once

#include <QSortFilterProxyModel>
#include "database/schema.h"

class CellFilteredProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

private:
    int columnIndex;

public:
    QVariant data;

    CellFilteredProxyModel(QObject *parent, QVariant data, int columnIndex = 0) : QSortFilterProxyModel(parent),
        columnIndex(columnIndex), data(data) {
    };

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};
