//
// Created by niko on 29/09/24.
//

#include <QComboBox>
#include <QLineEdit>
#include <QSqlRecord>

#include "proxy_enabled_relational_delegate.h"

#include "custom_sql_relational_model.h"
#include "model_utils.h"

QWidget *SuperSqlRelationalDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                                  const QModelIndex &index) const {
    const QAbstractProxyModel *proxyModel = qobject_cast<const QAbstractProxyModel *>(index.model());
    if (proxyModel == nullptr) {
        // Nothing to do for us.
        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    auto *sourceModel = find_source_model_of_type<CustomSqlRelationalModel>(proxyModel);
    if (sourceModel == nullptr) {
        // Nothing to do for us.
        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    QSqlTableModel *childModel = sourceModel->relationModel(index.column());
    if (childModel == nullptr) {
        // Nothing to do for us.
        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    auto *comboBox = new QComboBox(parent);
    comboBox->setEditable(true);
    comboBox->setModel(childModel);
    comboBox->setModelColumn(sourceModel->relation(index.column()).displayColumn());
    comboBox->installEventFilter(const_cast<SuperSqlRelationalDelegate *>(this));

    return comboBox;
}

void
SuperSqlRelationalDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    if (!index.isValid()) {
        return;
    }

    const QAbstractProxyModel *proxyModel = qobject_cast<const QAbstractProxyModel *>(index.model());
    if (proxyModel == nullptr) {
        // Nothing to do for us.
        QStyledItemDelegate::setModelData(editor, model, index);
        return;
    }

    auto *sqlModel = find_source_model_of_type<CustomSqlRelationalModel>(proxyModel);
    if (sqlModel == nullptr) {
        // Nothing to do for us.
        QStyledItemDelegate::setModelData(editor, model, index);
        return;
    }

    QSqlTableModel *childModel = sqlModel->relationModel(index.column());
    if (childModel == nullptr) {
        // Nothing to do for us.
        QStyledItemDelegate::setModelData(editor, model, index);
        return;
    }

    auto combo = qobject_cast<QComboBox *>(editor);
    if (combo == nullptr) {
        // Nothing to do for us.
        QStyledItemDelegate::setModelData(editor, model, index);
        return;
    }
    int currentItem = combo->currentIndex();
    // Column that is displayed in the combobox.
    int childColIndex = sqlModel->relation(index.column()).displayColumn();
    // Column with the primary key of the combobox.
    int childEditIndex = sqlModel->relation(index.column()).primaryKeyColumn();

    // The insertion should have been done, probably?

    auto currentText = combo->lineEdit()->text();
    auto itemText = childModel->index(currentItem, childColIndex).data(Qt::DisplayRole);

    // If the current text does not match the item's text, we must insert a new item instead.
    // TODO: how to handle errors here?
    if (currentText != itemText) {
        auto newRecord = childModel->record();
        newRecord.setGenerated(childEditIndex, true);
        newRecord.setValue(childColIndex, currentText);
        currentItem = childModel->rowCount();
        // TODO: how to handle errors here?
        if (!childModel->insertRecord(currentItem, newRecord)) {
            return;
        }
        // TODO: why is this not automatic, and how to handle errors?
        childModel->submitAll();
        childModel->select();
    }

    model->setData(index,
                   childModel->index(currentItem, childColIndex).data(Qt::DisplayRole),
                   Qt::DisplayRole);
    model->setData(index,
                   childModel->index(currentItem, childEditIndex).data(Qt::EditRole),
                   Qt::EditRole);
}
