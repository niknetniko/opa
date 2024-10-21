//
// Created by niko on 17/10/24.
//

#ifndef CUSTOM_SQL_RELATIONAL_MODEL_H
#define CUSTOM_SQL_RELATIONAL_MODEL_H

#include <QSqlTableModel>

#include "database/schema.h"

class ForeignKey {
public:
    /**
     * Create an invalid foreign key.
     */
    ForeignKey() = default;

    /**
     * Create a foreign key which should show the given column in the model.
     *
     * @param foreignKeyColumn The column in the original model that contains the foreign key.
     * @param foreignModel The model for accessing the foreign table.
     * @param displayColumn The column in the foreign model to use for display purposes.
     * @param primaryKeyColumn The column in the foreign model that contains the primary key.
     */
    ForeignKey(int foreignKeyColumn, QSqlTableModel *foreignModel, int displayColumn, int primaryKeyColumn);

    QSqlTableModel *foreignModel() const;

    int displayColumn() const;

    int foreignKeyColumn() const;

    int primaryKeyColumn() const;

    /**
     * Returns true if this foreign key is valid, false otherwise.
     */
    bool isValid() const;

private:
    int _displayColumn = -1;
    int _primaryKeyColumn = -1;
    int _foreignKeyColumn = -1;

    QSqlTableModel *_foreignModel = nullptr;
};

/**
 * A custom version of the QSqlRelationalTableModel.
 * The biggest differences are:
 *
 * - The data from the foreign key is added as an additional table.
 * - The model gets the data for the foreign tables from an existing user-provided model.
 *
 * The implementation of this model is based on both QSqlRelationalTableModel and KExtraColumnsProxyModel.
 *
 * Note that the extra columns are not addressable by name at the moment.
 * Additionally, the extra columns are not editable and are not present in any SqlRecord.
 */
class CustomSqlRelationalModel : public QSqlTableModel {
    Q_OBJECT

public:
    explicit CustomSqlRelationalModel(QObject *parent = nullptr);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * Get the foreign key relation for a column.
     *
     * If the column is not the result of a foreign relation, an invalid relation
     * will be returned.
     *
     * @param column The absolute index of the extra column.
     * @return The relation, valid or invalid depending on the column.
     */
    ForeignKey relation(int column) const;

    /**
     * Get the foreign model for a column.
     *
     * If the column is not the result of a foreign relation, null will be returned.
     *
     * @param column The absolute index of the extra column.
     * @return A pointer to the model or null if none.
     */
    QSqlTableModel *relationModel(int column) const;


    /**
     * Set the relation for a certain column.
     *
     * This indicates that foreignKeyColumn contains a foreign key to the given foreign model.
     * The foreignKeyColumn refers to the sourceModel column in the foreign model.
     * An additional column with the displayColumn (in the foreign model) will be added to this model.
     *
     * You must ensure that the foreign table model lives at least as long as this model, but this model
     * does not formally take ownership of the foreign model.
     *
     * @param foreignKeyColumn The column in this model that refers to the foreign table.
     * @param foreignModel The model for the foreign table.
     * @param displayColumn The column in the foreign table to add as an extra column to this model.
     * @param sourceModel The column in the foreign table the foreignKeyColumn refers to.
     */
    void setRelation(int foreignKeyColumn, QSqlTableModel *foreignModel,
                     int displayColumn, int sourceModel);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QModelIndex buddy(const QModelIndex &index) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

public Q_SLOTS:
    /**
     * Construct a new value cache for the given model.
     *
     * @param extraColumn The extra column number (e.g. the first column starts at 0).
     */
    void constructForeignCache(int extraColumn);

private:
    QVector<ForeignKey> _foreignKeys;
    QVector<QHash<IntegerPrimaryKey, QPersistentModelIndex> > _foreignValues;

    int asExtraColumn(int column) const;

    QPair<const ForeignKey &, int> getFkFromForeignKeyColumn(int column) const;
};

#endif //CUSTOM_SQL_RELATIONAL_MODEL_H
