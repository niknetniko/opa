/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "proxy_enabled_relational_delegate.h"

#include "custom_sql_relational_model.h"
#include "model_utils_find_source_model_of_type.h"

#include <QComboBox>
#include <QLineEdit>
#include <QSqlError>

SuperSqlRelationalDelegate::SuperSqlRelationalDelegate(QObject* parent) : QStyledItemDelegate(parent) {
}

QWidget* SuperSqlRelationalDelegate::createEditor(
    QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index
) const {
    const auto* proxyModel = qobject_cast<const QAbstractProxyModel*>(index.model());
    if (proxyModel == nullptr) {
        // Nothing to do for us.
        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    const auto* sourceModel = findSourceModelOfType<CustomSqlRelationalModel>(proxyModel);
    if (sourceModel == nullptr) {
        // Nothing to do for us.
        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    QSqlTableModel* childModel = sourceModel->relationModel(index.column());
    if (childModel == nullptr) {
        // Nothing to do for us.
        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    auto* comboBox = new QComboBox(parent);
    comboBox->setEditable(true);
    comboBox->setModel(childModel);
    comboBox->setModelColumn(sourceModel->relation(index.column()).displayColumn());
    comboBox->installEventFilter(const_cast<SuperSqlRelationalDelegate*>(this));

    return comboBox;
}

void SuperSqlRelationalDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index)
    const {
    if (!index.isValid()) {
        return;
    }

    const auto* proxyModel = qobject_cast<const QAbstractProxyModel*>(model);
    if (proxyModel == nullptr) {
        // Nothing to do for us.
        QStyledItemDelegate::setModelData(editor, model, index);
        return;
    }

    auto* sqlModel = const_cast<CustomSqlRelationalModel*>(findSourceModelOfType<CustomSqlRelationalModel>(proxyModel));
    if (sqlModel == nullptr) {
        // Nothing to do for us.
        QStyledItemDelegate::setModelData(editor, model, index);
        return;
    }
    auto sqlIndex = mapToSourceModel(index);
    Q_ASSERT(sqlModel->checkIndex(sqlIndex));
    qDebug() << "For SQL model, index is" << index << "->" << sqlIndex;

    QSqlTableModel* childModel = sqlModel->relationModel(sqlIndex.column());
    if (childModel == nullptr) {
        // Nothing to do for us.
        QStyledItemDelegate::setModelData(editor, model, index);
        return;
    }

    auto* combo = qobject_cast<QComboBox*>(editor);
    if (combo == nullptr) {
        // Nothing to do for us.
        QStyledItemDelegate::setModelData(editor, model, index);
        return;
    }
    const int currentItem = combo->currentIndex();

    // When the user enters new data, ensure it is committed to the database.
    if (!childModel->submitAll()) {
        qWarning() << "Could not add new data for child model in delegate!";
        qWarning() << childModel->lastError();
        return;
    }

    // Column with the primary key of the combobox.
    const int childEditIndex = sqlModel->relation(sqlIndex.column()).primaryKeyColumn();

    // Both are the same, so update the foreign key column instead.
    const int fkColumn = sqlModel->relation(sqlIndex.column()).foreignKeyColumn();
    // Index for the fk in the parent model.
    auto fkIndex = sqlModel->index(sqlIndex.row(), fkColumn);
    // Index for the pk in the foreign model.
    auto pkIndex = childModel->index(currentItem, childEditIndex);
    sqlModel->setData(fkIndex, pkIndex.data(Qt::DisplayRole), Qt::DisplayRole);
    sqlModel->setData(fkIndex, pkIndex.data(Qt::EditRole), Qt::EditRole);
}
