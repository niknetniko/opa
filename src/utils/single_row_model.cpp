#include "single_row_model.h"

CellFilteredProxyModel::CellFilteredProxyModel(QObject *parent) : QSortFilterProxyModel(parent) {
}

void CellFilteredProxyModel::addFilter(int columnIndex, const QVariant &data) {
    this->filters.append({.column = columnIndex, .data = data});
}

bool CellFilteredProxyModel::filterAcceptsRow(int source_row, const QModelIndex &sourceParent)
    const {
    for (auto [columnIndex, data]: this->filters) {
        auto index = this->sourceModel()->index(source_row, columnIndex, sourceParent);
        if (index.data() != data) {
            return false;
        }
    }

    return true;
}
