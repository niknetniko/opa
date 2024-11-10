/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "custom_sql_relational_model.h"

#include "model_utils_find_source_model_of_type.h"

#include <QIdentityProxyModel>

ForeignKey::ForeignKey(
    const int foreignKeyColumn, QSqlTableModel* foreignModel, const int displayColumn, const int primaryKeyColumn
) :
    _displayColumn(displayColumn),
    _primaryKeyColumn(primaryKeyColumn),
    _foreignKeyColumn(foreignKeyColumn),
    _foreignModel(foreignModel) {
    Q_ASSERT(0 <= displayColumn);
    Q_ASSERT(displayColumn < foreignModel->columnCount());
    Q_ASSERT(0 <= primaryKeyColumn);
    Q_ASSERT(primaryKeyColumn < foreignModel->columnCount());
}

QSqlTableModel* ForeignKey::foreignModel() const {
    return this->_foreignModel;
}

int ForeignKey::displayColumn() const {
    return this->_displayColumn;
}

int ForeignKey::foreignKeyColumn() const {
    return this->_foreignKeyColumn;
}

int ForeignKey::primaryKeyColumn() const {
    return this->_primaryKeyColumn;
}

bool ForeignKey::isValid() const {
    return this->_displayColumn != -1 && this->_foreignModel != nullptr && this->_foreignKeyColumn != -1 &&
           this->_primaryKeyColumn != -1;
}

CustomSqlRelationalModel::CustomSqlRelationalModel(QObject* parent) : QSqlTableModel(parent) {
}

int CustomSqlRelationalModel::columnCount(const QModelIndex& parent) const {
    Q_ASSERT(checkIndex(parent, CheckIndexOption::ParentIsInvalid));
    return QSqlTableModel::columnCount(parent) + static_cast<int>(this->_foreignKeys.count());
}

ForeignKey CustomSqlRelationalModel::relation(const int column) const {
    Q_ASSERT(0 <= column);
    const int extraColumn = asExtraColumn(column);
    if (extraColumn < 0) {
        return {};
    }
    return this->_foreignKeys.at(extraColumn);
}

QSqlTableModel* CustomSqlRelationalModel::relationModel(const int column) const {
    Q_ASSERT(0 <= column);
    return relation(column).foreignModel();
}

void CustomSqlRelationalModel::setRelation(
    const int foreignKeyColumn, QSqlTableModel* foreignModel, const int displayColumn, const int sourceModel
) {
    Q_ASSERT(0 <= foreignKeyColumn);
    Q_ASSERT(foreignKeyColumn < QSqlTableModel::columnCount());

    const int extraColumn = static_cast<int>(_foreignKeys.count());
    this->_foreignKeys.append(ForeignKey(foreignKeyColumn, foreignModel, displayColumn, sourceModel));

    // Construct the dictionary of primary keys to values for easy access later.
    this->_foreignValues.append(QHash<IntegerPrimaryKey, QPersistentModelIndex>());
    Q_ASSERT(this->_foreignKeys.count() == this->_foreignValues.count());
    this->constructForeignCache(extraColumn);

    // Listen to updates for this model.
    connect(foreignModel, &QAbstractItemModel::dataChanged, this, [this, extraColumn] {
        this->constructForeignCache(extraColumn);
    });
    connect(foreignModel, &QAbstractItemModel::rowsInserted, this, [this, extraColumn] {
        this->constructForeignCache(extraColumn);
    });
    connect(foreignModel, &QAbstractItemModel::rowsRemoved, this, [this, extraColumn] {
        this->constructForeignCache(extraColumn);
    });
    connect(foreignModel, &QAbstractItemModel::modelReset, this, [this, extraColumn] {
        this->constructForeignCache(extraColumn);
    });
}

QModelIndex CustomSqlRelationalModel::index(const int row, const int column, const QModelIndex& parent) const {
    // Taken from KExtraColumnProxyModel
    if (const int extraCol = asExtraColumn(column); extraCol >= 0) {
        // We store the internal pointer of the index for column 0 in the proxy index for extra
        // columns. This will be useful in the parent method.
        return createIndex(row, column, QSqlTableModel::index(row, 0, parent).internalPointer());
    }
    return QSqlTableModel::index(row, column, parent);
}

QVariant CustomSqlRelationalModel::data(const QModelIndex& index, const int role) const {
    Q_ASSERT(checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));

    if (const int extraColumn = asExtraColumn(index.column()); extraColumn >= 0) {
        const auto fkObject = this->_foreignKeys.at(extraColumn);
        const auto fkValue = this->index(index.row(), fkObject.foreignKeyColumn()).data(role).toLongLong();
        return this->_foreignValues.at(extraColumn)[fkValue].data(role);
    }

    return QSqlTableModel::data(index, role);
}

QModelIndex CustomSqlRelationalModel::buddy(const QModelIndex& index) const {
    if (const int extra = asExtraColumn(index.column()); extra >= 0) {
        return index;
    }

    return QSqlTableModel::buddy(index);
}

bool CustomSqlRelationalModel::setData(const QModelIndex& index, const QVariant& value, const int role) {
    Q_ASSERT(checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));

    if (const int extraCol = asExtraColumn(index.column()); extraCol >= 0) {
        // Extra columns are read only.
        return false;
    }

    // If we are editing foreign keys, we also need to notify that the corresponding column has
    // changed.
    const bool result = QSqlTableModel::setData(index, value, role);
    if (auto [fk, extraColumn] = this->getFkFromForeignKeyColumn(index.column()); result && fk.isValid()) {
        auto changedIndex = this->index(index.row(), extraColumn, index.parent());
        Q_EMIT dataChanged(changedIndex, changedIndex, {role});
    }

    return result;
}

Qt::ItemFlags CustomSqlRelationalModel::flags(const QModelIndex& index) const {
    if (const int extraCol = asExtraColumn(index.column()); extraCol >= 0) {
        // Extra columns are readonly
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
    return QSqlTableModel::flags(index);
}

int CustomSqlRelationalModel::asExtraColumn(const int column) const {
    if (column >= QSqlTableModel::columnCount()) {
        return column - QSqlTableModel::columnCount();
    }
    return -1;
}

ForeignKeyAndColumn CustomSqlRelationalModel::getFkFromForeignKeyColumn(int column) const {
    for (int i = 0; i < _foreignKeys.length(); ++i) {
        if (_foreignKeys[i].foreignKeyColumn() == column) {
            return {.fk = _foreignKeys[i], .column = i};
        }
    }

    return {};
}

void connectComboBox(const QAbstractItemModel* model, int relationColumn, QComboBox* comboBox) {
    const auto* rootModel = findSourceModelOfType<CustomSqlRelationalModel>(model);
    QSqlTableModel* childModel = rootModel->relationModel(relationColumn);
    comboBox->setEditable(true);
    comboBox->setModel(childModel);
    comboBox->setModelColumn(rootModel->relation(relationColumn).displayColumn());
}

void CustomSqlRelationalModel::constructForeignCache(const int extraColumn) {
    const auto fk = this->_foreignKeys.at(extraColumn);
    auto* const foreignModel = fk.foreignModel();
    this->_foreignValues[extraColumn].clear();

    for (int r = 0; r < foreignModel->rowCount(); ++r) {
        auto pk = foreignModel->index(r, fk.primaryKeyColumn()).data();
        const auto value = foreignModel->index(r, fk.displayColumn()).data();
        this->_foreignValues[extraColumn][pk.toLongLong()] =
            QPersistentModelIndex(foreignModel->index(r, fk.displayColumn()));
    }

    // The data changed, so notify that extra column has been updated.
    const auto start = this->index(0, QSqlTableModel::columnCount() + extraColumn);
    const auto end = this->index(rowCount(), QSqlTableModel::columnCount() + extraColumn);
    Q_EMIT this->dataChanged(start, end);
}
