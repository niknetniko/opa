//
// Created by niko on 22/10/24.
//

#ifndef BUILTIN_DECORATION_MODEL_H
#define BUILTIN_DECORATION_MODEL_H
#include <KRearrangeColumnsProxyModel>
#include <QIdentityProxyModel>

/**
 * Model with support for built-in columns.
 *
 * The model will apply a few common things that are needed for working with tables with built-in stuff:
 *
 * - Show an icon on the main column for built-in rows.
 * - Hide the built-in column.
 * - Make built-in rows read-only.
 */
class BuiltinModel: public KRearrangeColumnsProxyModel {
    Q_OBJECT

public:
    explicit BuiltinModel(QObject* parent);

    void setColumns(int builtinColumn, int decoratedColumn);

    QVariant data(const QModelIndex &index, int role) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setSourceModel(QAbstractItemModel *sourceModel) override;

private:
    int decoratedColumn = -1;
    int builtinColumn = -1;

    void syncColumns();
};

#endif //BUILTIN_DECORATION_MODEL_H
