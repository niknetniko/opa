//
// Created by niko on 25/09/24.
//

#ifndef OPA_DATA_MANAGER_H
#define OPA_DATA_MANAGER_H

#include <QObject>
#include <QAbstractItemModel>
#include <QAbstractProxyModel>
#include <QSqlDatabase>
#include <QSqlRelationalTableModel>
#include "database/schema.h"

/**
 * The centralized point to get models for data access from the database.
 *
 * Unless otherwise noted, the models you get from this class will automatically updated if the underlying
 * data changes. This is mostly not the case for write models.
 *
 * You can also listen to the signal yourself and update things depending on the passed data.
 *
 * When getting a model, you need to pass in the parent; this will allow Qt to managing garbage collection.
 * If you pass in a parent that lives too shortly, the model will be removed too soon. Similarly, passing a parent that
 * lives too long will keep around the object too long.
 *
 * Some basic models are considered to be application-wide, so you do not have to pass in a parent.
 *
 * Unless otherwise noted, the model is not "loaded" with data, so call select() yourself.
 */
class DataManager : public QObject {
Q_OBJECT

public:
    // TODO: remove this once no longer needed.
    static DataManager *getInstance(QObject* parent);

    QSqlTableModel *namesModel() const;

    /**
     * Get a model representing all names for a single person.
     *
     * @param personId The ID of the person to filter on.
     */
    QAbstractProxyModel *namesModelForPerson(QObject *parent, IntegerPrimaryKey personId);

    /**
     * Model for a single name.
     *
     * @param nameId The ID of the name.
     */
    QAbstractProxyModel *singleNameModel(QObject *parent, IntegerPrimaryKey nameId);

    /**
     * Model showing only the primary names for each person.
     *
     * @param parent The parent of the model.
     */
    QAbstractProxyModel *primaryNamesModel(QObject *parent);

    /**
     * Model for the details view of a person.
     */
     QAbstractProxyModel *personDetailsModel(QObject* parent, IntegerPrimaryKey personId);

Q_SIGNALS:
    void dataChanged(QString table);

public Q_SLOTS:
    void onNamesTableChanged();

private:
    explicit DataManager(QObject *parent);

    static DataManager *instance;

    QSqlRelationalTableModel * baseNamesModel;
};

#endif //OPA_DATA_MANAGER_H
