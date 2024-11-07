#include "edit_proxy_model.h"

EditProxyModel::EditProxyModel(QObject *parent) : QIdentityProxyModel(parent) {
}

void EditProxyModel::addReadOnlyColumns(const QList<int> &columns) {
    this->columns.append(columns);
}

Qt::ItemFlags EditProxyModel::flags(const QModelIndex &index) const {
    auto flags = QIdentityProxyModel::flags(index);
    if (!index.isValid()) {
        return flags;
    }

    if (columns.contains(index.column())) {
        flags = flags.setFlag(Qt::ItemIsEditable, false);
    }
    return flags;
}
