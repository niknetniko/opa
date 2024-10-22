//
// Created by niko on 22/10/24.
//

#include "builtin_model.h"

#include <QIcon>

BuiltinModel::BuiltinModel(QObject* parent): KRearrangeColumnsProxyModel(parent) {
}

void BuiltinModel::setColumns(int builtinColumn, int decoratedColumn) {
    this->builtinColumn = builtinColumn;
    this->decoratedColumn = decoratedColumn;
    this->syncColumns();
}

QVariant BuiltinModel::data(const QModelIndex &index, int role) const {
    Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid));

    auto sourceIndex = mapToSource(index);
    auto builtinIndex = sourceModel()->index(sourceIndex.row(), this->builtinColumn);
    Q_ASSERT(sourceModel()->checkIndex(builtinIndex, CheckIndexOption::IndexIsValid));

    if (sourceIndex.column() == this->decoratedColumn && role == Qt::DecorationRole && builtinIndex.data().toBool()) {
        return QIcon::fromTheme(QStringLiteral("lock"));
    }

    return QIdentityProxyModel::data(index, role);
}

Qt::ItemFlags BuiltinModel::flags(const QModelIndex &index) const {
    auto flags = QIdentityProxyModel::flags(index);

    if (!index.isValid()) {
        return flags;
    }

    auto sourceIndex = mapToSource(index);
    auto builtinIndex = sourceModel()->index(sourceIndex.row(), this->builtinColumn);
    Q_ASSERT(sourceModel()->checkIndex(builtinIndex, CheckIndexOption::IndexIsValid));

    if (builtinIndex.data().toBool()) {
        flags = flags & ~Qt::ItemIsEditable;
    }
    return flags;
}

void BuiltinModel::setSourceModel(QAbstractItemModel *sourceModel) {
    KRearrangeColumnsProxyModel::setSourceModel(sourceModel);
    this->syncColumns();
}

void BuiltinModel::syncColumns() {
    if (this->builtinColumn == -1 || this->sourceModel() == nullptr) {
        return;
    }

    QList<int> columns(QIdentityProxyModel::columnCount());
    std::iota(columns.begin(), columns.end(), 0);
    columns.removeAt(this->builtinColumn);
    this->setSourceColumns(columns);
}
