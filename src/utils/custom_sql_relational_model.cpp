//
// Created by niko on 17/10/24.
//

#include "custom_sql_relational_model.h"

#include <QIdentityProxyModel>

ForeignKey::ForeignKey(const int foreignKeyColumn, QSqlTableModel *foreignModel, const int displayColumn,
                       const int primaryKeyColumn): _displayColumn(displayColumn), _primaryKeyColumn(primaryKeyColumn),
                                                    _foreignKeyColumn(foreignKeyColumn), _foreignModel(foreignModel) {
    Q_ASSERT(0 <= displayColumn);
    Q_ASSERT(displayColumn < foreignModel->columnCount());
    Q_ASSERT(0 <= primaryKeyColumn);
    Q_ASSERT(primaryKeyColumn < foreignModel->columnCount());
}

QSqlTableModel *ForeignKey::foreignModel() const {
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

CustomSqlRelationalModel::CustomSqlRelationalModel(QObject *parent): QSqlTableModel(parent) {
}

int CustomSqlRelationalModel::columnCount(const QModelIndex &parent) const {
    Q_ASSERT(checkIndex(parent, CheckIndexOption::ParentIsInvalid));
    return QSqlTableModel::columnCount(parent) + this->_foreignKeys.count();
}

ForeignKey CustomSqlRelationalModel::relation(const int column) const {
    Q_ASSERT(0 <= column);
    return this->_foreignKeys.at(column);
}

QSqlTableModel *CustomSqlRelationalModel::relationModel(const int column) const {
    Q_ASSERT(0 <= column);
    return this->_foreignKeys.at(column).foreignModel();
}

void CustomSqlRelationalModel::setRelation(const int foreignKeyColumn, QSqlTableModel *foreignModel,
                                           const int displayColumn, const int sourceModel) {
    Q_ASSERT(0 <= foreignKeyColumn);
    Q_ASSERT(foreignKeyColumn < QSqlTableModel::columnCount());

    const int extraColumn = this->_foreignKeys.count();
    this->_foreignKeys.append(ForeignKey(foreignKeyColumn, foreignModel, displayColumn, sourceModel));

    // Construct the dictionary of primary keys to values for easy access later.
    this->_foreignValues.append(QHash<IntegerPrimaryKey, QVariant>());
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

QModelIndex CustomSqlRelationalModel::index(const int row, const int column, const QModelIndex &parent) const {
    // Taken from KExtraColumnProxyModel
    if (const int extraCol = asExtraColumn(column); extraCol >= 0) {
        // We store the internal pointer of the index for column 0 in the proxy index for extra columns.
        // This will be useful in the parent method.
        return createIndex(row, column, QSqlTableModel::index(row, 0, parent).internalPointer());
    }
    return QSqlTableModel::index(row, column, parent);
}

QVariant CustomSqlRelationalModel::data(const QModelIndex &index, const int role) const {
    Q_ASSERT(checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));

    if (const int extraColumn = asExtraColumn(index.column()); extraColumn >= 0) {
        const auto fkObject = this->_foreignKeys.at(extraColumn);
        const auto fkValue = this->index(index.row(), fkObject.foreignKeyColumn()).data(role).toLongLong();
        return this->_foreignValues.at(extraColumn)[fkValue];
    }

    return QSqlTableModel::data(index, role);
}

QModelIndex CustomSqlRelationalModel::buddy(const QModelIndex &index) const {
    if (const int extra = asExtraColumn(index.column()); extra >= 0) {
        return index;
    }

    return QSqlTableModel::buddy(index);
}

bool CustomSqlRelationalModel::setData(const QModelIndex &index, const QVariant &value, const int role) {
    Q_ASSERT(checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));

    if (const int extraCol = asExtraColumn(index.column()); extraCol >= 0) {
        // Extra columns are read only.
        return false;
    }

    return QSqlTableModel::setData(index, value, role);
}

Qt::ItemFlags CustomSqlRelationalModel::flags(const QModelIndex &index) const {
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

void CustomSqlRelationalModel::constructForeignCache(const int extraColumn) {
    const auto fk = this->_foreignKeys.at(extraColumn);
    const auto foreignModel = fk.foreignModel();
    this->_foreignValues[extraColumn].clear();

    for (int r = 0; r < foreignModel->rowCount(); ++r) {
        auto pk = foreignModel->index(r, fk.primaryKeyColumn()).data();
        const auto value = foreignModel->index(r, fk.displayColumn()).data();
        this->_foreignValues[extraColumn][pk.toLongLong()] = value;
    }

    // The data changed, so notify that extra column has been updated.
    const auto start = this->index(0, QSqlTableModel::columnCount() + extraColumn);
    const auto end = this->index(rowCount(), QSqlTableModel::columnCount() + extraColumn);
    Q_EMIT this->dataChanged(start, end);
}
