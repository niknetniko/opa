
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "genealogical_date_proxy_model.h"

#include "genealogical_date.h"

GenealogicalDateProxyModel::GenealogicalDateProxyModel(QObject* parent) : QIdentityProxyModel(parent) {
}

int GenealogicalDateProxyModel::dateColumn() const {
    return this->dateColumn_;
}

void GenealogicalDateProxyModel::setDateColumn(int column) {
    // Verify that the columns exist.
    assert(0 <= column);
    assert(column < this->columnCount());
    this->dateColumn_ = column;
}

QVariant GenealogicalDateProxyModel::data(const QModelIndex& index, int role) const {
    // TODO: enable this assertion...
    // Q_ASSERT(checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));
    if (index.column() == this->dateColumn_) {
        switch (role) {
            case Qt::DisplayRole:
            case Qt::EditRole: {
                auto raw = QIdentityProxyModel::data(index, role).toString();
                return GenealogicalDate::fromDatabaseRepresentation(raw).toDisplayText();
            }
            case RawDateRole: {
                auto raw = QIdentityProxyModel::data(index, Qt::DisplayRole).toString();
                return QVariant::fromValue(GenealogicalDate::fromDatabaseRepresentation(raw));
            }
            default:
                return QIdentityProxyModel::data(index, role);
        }
    }

    return QIdentityProxyModel::data(index, role);
}

bool GenealogicalDateProxyModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    // TODO: enable this assertion...
    // Q_ASSERT(checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));
    QVariant savedValue = value;
    int parentRole = role;
    if (index.column() == this->dateColumn_) {
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            savedValue = GenealogicalDate::fromDisplayText(value.toString()).toDatabaseRepresentation();
        } else if (role == RawDateRole) {
            savedValue = value.value<GenealogicalDate>().toDatabaseRepresentation();
            parentRole = Qt::EditRole;
        }
    }

    return QIdentityProxyModel::setData(index, savedValue, parentRole);
}
