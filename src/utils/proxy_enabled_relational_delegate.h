#pragma once

#include <QStyledItemDelegate>

/**
 * Version of a QSqlRelationalDelegate that works with CustomSqlRelationalModel, with support for proxy models.
 * It also supports the following:
 *
 * - Supports working through proxy models, by using "findSourceModelOfType".
 * - If an item to set does not exist in the other table, it will be added, and then linked to the current table.
 *
 */
class SuperSqlRelationalDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit SuperSqlRelationalDelegate(QObject *parent);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};
