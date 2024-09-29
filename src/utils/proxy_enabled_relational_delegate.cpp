//
// Created by niko on 29/09/24.
//

#include <QSqlRelationalTableModel>
#include <QComboBox>
#include <QLineEdit>
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlQuery>
#include "proxy_enabled_relational_delegate.h"
#include "model_utils.h"

QWidget *SuperSqlRelationalDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                                  const QModelIndex &index) const {
    const QAbstractProxyModel *proxyModel = qobject_cast<const QAbstractProxyModel *>(index.model());
    if (proxyModel == nullptr) {
        // Nothing to do for us.
        return QSqlRelationalDelegate::createEditor(parent, option, index);
    }

    auto *sourceModel = find_source_model_of_type<QSqlRelationalTableModel>(proxyModel);
    if (sourceModel == nullptr) {
        // Nothing to do for us.
        return QSqlRelationalDelegate::createEditor(parent, option, index);
    }

    QSqlTableModel *childModel = sourceModel->relationModel(index.column());
    if (childModel == nullptr) {
        // Nothing to do for us.
        return QSqlRelationalDelegate::createEditor(parent, option, index);
    }

    auto *comboBox = new QComboBox(parent);
    comboBox->setEditable(true);
    comboBox->setModel(childModel);
    comboBox->setModelColumn(childModel->fieldIndex(sourceModel->relation(index.column()).displayColumn()));
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
        QSqlRelationalDelegate::setModelData(editor, model, index);
        return;
    }

    auto *sourceModel = find_source_model_of_type<QSqlRelationalTableModel>(proxyModel);
    if (sourceModel == nullptr) {
        // Nothing to do for us.
        QSqlRelationalDelegate::setModelData(editor, model, index);
        return;
    }
    auto *sqlModel = const_cast<QSqlRelationalTableModel *>(sourceModel);

    QSqlTableModel *childModel = sqlModel->relationModel(index.column());
    if (childModel == nullptr) {
        // Nothing to do for us.
        QSqlRelationalDelegate::setModelData(editor, model, index);
        return;
    }

    QComboBox *combo = qobject_cast<QComboBox *>(editor);
    if (combo == nullptr) {
        // Nothing to do for us.
        QSqlRelationalDelegate::setModelData(editor, model, index);
        return;
    }

    int currentItem = combo->currentIndex();
    int childColIndex = childModel->fieldIndex(sqlModel->relation(index.column()).displayColumn());
    int childEditIndex = childModel->fieldIndex(sqlModel->relation(index.column()).indexColumn());
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
