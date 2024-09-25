//
// Created by niko on 15/09/24.
//
#include <QSqlRelationalTableModel>
#include "single_row_model.h"

bool CellFilteredProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    auto index = this->sourceModel()->index(source_row, this->columnIndex, source_parent);
//    qDebug() << "For row " << source_row << ", selected index " << index << ", against " << this->data;
//    qDebug() << "  index data is now " << index.data(Qt::EditRole);
    return index.data(Qt::EditRole) == this->data;
}

bool CellFilteredProxyModel::submit() {
    bool result = QSortFilterProxyModel::submit();
    QSqlRelationalTableModel *sourceModel = qobject_cast<QSqlRelationalTableModel*>(this->sourceModel());
    if (sourceModel) {
        result &= sourceModel->submitAll();
    }
    return result;
}
