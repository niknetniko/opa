/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "read_only_proxy_model.h"

ReadOnlyProxyModel::ReadOnlyProxyModel(QObject* parent) : QIdentityProxyModel(parent) {
}

void ReadOnlyProxyModel::addReadOnlyColumns(const QList<int>& columns) {
    this->columns.append(columns);
}

Qt::ItemFlags ReadOnlyProxyModel::flags(const QModelIndex& index) const {
    auto flags = QIdentityProxyModel::flags(index);

    if (columns.contains(index.column())) {
        flags = flags.setFlag(Qt::ItemIsEditable, false);
    }
    return flags;
}
