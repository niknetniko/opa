//
// Created by niko on 29/09/24.
//

#ifndef OPA_PROXY_ENABLED_RELATIONAL_DELEGATE_H
#define OPA_PROXY_ENABLED_RELATIONAL_DELEGATE_H

#include <QStyledItemDelegate>

/**
 * Version of a QSqlRelationalDelegate that works with CustomSqlRelationalModel, with support for proxy models.
 * It also supports the following:
 *
 * - Supports working through proxy models, by using "find_source_model_of_type".
 * - If an item to set does not exist in the other table, it will be added, and then linked to the current table.
 *
 */
class SuperSqlRelationalDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit SuperSqlRelationalDelegate(QObject *parent) : QStyledItemDelegate(parent) {
    };

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};


#endif //OPA_PROXY_ENABLED_RELATIONAL_DELEGATE_H
