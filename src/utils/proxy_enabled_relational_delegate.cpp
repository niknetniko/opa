//
// Created by niko on 29/09/24.
//

#include <QComboBox>
#include <QLineEdit>

#include "proxy_enabled_relational_delegate.h"

#include <QSqlError>

#include "custom_sql_relational_model.h"
#include "model_utils.h"

QWidget *SuperSqlRelationalDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                                  const QModelIndex &index) const {
    auto proxyModel = qobject_cast<const QAbstractProxyModel *>(index.model());
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

    auto proxyModel = qobject_cast<const QAbstractProxyModel *>(model);
    if (proxyModel == nullptr) {
        // Nothing to do for us.
        QStyledItemDelegate::setModelData(editor, model, index);
        return;
    }

    auto sqlModel = const_cast<CustomSqlRelationalModel *>(find_source_model_of_type<CustomSqlRelationalModel>(proxyModel));
    if (sqlModel == nullptr) {
        // Nothing to do for us.
        QStyledItemDelegate::setModelData(editor, model, index);
        return;
    }
    auto sqlIndex = map_to_source_model(index);
    Q_ASSERT(sqlModel->checkIndex(sqlIndex));
    qDebug() << "For SQL model, index is" << index << "->" << sqlIndex;

    QSqlTableModel *childModel = sqlModel->relationModel(sqlIndex.column());
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

    // When the user enters new data, ensure it is committed to the database.
    if (!childModel->submitAll()) {
        qWarning() << "Could not add new data for child model in delegate!";
        qWarning() << childModel->lastError();
        return;
    }

    // Column with the primary key of the combobox.
    int childEditIndex = sqlModel->relation(sqlIndex.column()).primaryKeyColumn();

    // Both are the same, so update the foreign key column instead.
    int fkColumn = sqlModel->relation(sqlIndex.column()).foreignKeyColumn();
    // Index for the fk in the parent model.
    auto fkIndex = sqlModel->index(sqlIndex.row(), fkColumn);
    // Index for the pk in the foreign model.
    auto pkIndex = childModel->index(currentItem, childEditIndex);
    sqlModel->setData(fkIndex, pkIndex.data(Qt::DisplayRole), Qt::DisplayRole);
    sqlModel->setData(fkIndex, pkIndex.data(Qt::EditRole), Qt::EditRole);
}
