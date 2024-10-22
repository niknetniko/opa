//
// Created by niko on 22/10/24.
//

#include "edit_proxy_model.h"

EditProxyModel::EditProxyModel(QObject *parent): QIdentityProxyModel(parent) {
}

void EditProxyModel::addReadOnlyColumns(const QList<int> &columns) {
    this->columns.append(columns);
}

Qt::ItemFlags EditProxyModel::flags(const QModelIndex &index) const {
    auto flags = QIdentityProxyModel::flags(index);
    if (columns.contains(index.column())) {
        flags = flags & ~Qt::ItemIsEditable;
    }
    return flags;
}
