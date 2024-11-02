#pragma once

#include <QSortFilterProxyModel>
#include "database/schema.h"


struct ColumnAndDataPair {
    int column;
    QVariant data;
};

class CellFilteredProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit CellFilteredProxyModel(QObject *parent);

    void addFilter(int columnIndex, const QVariant &data);

    bool filterAcceptsRow(int source_row, const QModelIndex &sourceParent) const override;

private:
    QList<ColumnAndDataPair> filters;
};
