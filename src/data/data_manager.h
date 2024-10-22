#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include <QAbstractProxyModel>
#include <QObject>
#include <QSqlRelationalTableModel>

#include "database/schema.h"
#include "utils/custom_sql_relational_model.h"

template<class M>
concept QSqlTableModelConcept = std::is_base_of_v<QSqlTableModel, M>;

/**
 * The centralized point to get models for data access from the database.
 */
class DataManager : public QObject {
    Q_OBJECT

public:
    static void initialize(QObject *parent);

    static DataManager &get();

    QSqlTableModel *namesModel() const;

    QSqlTableModel *nameOriginsModel() const;

    QSqlTableModel *eventRolesModel() const;

    QSqlTableModel *eventTypesModel() const;

    QSqlTableModel *eventRelationsModel() const;

    QSqlTableModel *eventsModel() const;

    /**
     * Get a model representing all names for a single person.
     *
     * @param parent The parent for the returned model.
     * @param personId The ID of the person to filter on.
     */
    QAbstractProxyModel *namesModelForPerson(QObject *parent, IntegerPrimaryKey personId) const;

    /**
     * Model for a single name.
     *
     * @param parent The parent for the returned model.
     * @param nameId The ID of the name.
     */
    QAbstractProxyModel *singleNameModel(QObject *parent, IntegerPrimaryKey nameId) const;

    /**
     * Model showing only the primary names for each person.
     *
     * @param parent The parent of the model.
     */
    QAbstractProxyModel *primaryNamesModel(QObject *parent);

    /**
     * Model for the details view of a person.
     */
    QAbstractProxyModel *personDetailsModel(QObject *parent, IntegerPrimaryKey personId);

    QAbstractProxyModel *eventsModelForPerson(QObject *parent, IntegerPrimaryKey personId);

Q_SIGNALS:
    /**
     * Called when a change has occurred in a certain model, and other models depending on data
     * from this table should also update.
     *
     * @param table
     */
    void dataChanged(QString table);

public Q_SLOTS:
    /** See listenToModel */
    void onSourceModelChanged();

private:
    static DataManager *instance;

    explicit DataManager(QObject *parent);

    /**
     * This is set to an address if the model updates are being sent from
     * the DataManager, otherwise it origines from the model itself.
     * Its main use is preventing signal loops, so if set to an address,
     * the updates will not propagate to the DataManager.
     */
    void *updatingFromDataManagerSource = nullptr;

    // Base models that allow editing.
    CustomSqlRelationalModel *baseNamesModel;
    QSqlTableModel *baseNameOriginModel;
    QSqlTableModel *baseEventRolesModel;
    QSqlTableModel *baseEventTypesModel;
    CustomSqlRelationalModel *baseEventRelationsModel;
    CustomSqlRelationalModel *baseEventsModel;

    /**
     * Connect the given model to the DataManager. All updates in
     * the model will trigger the update of the dataChanged signal.
     *
     * @param model The model to listen to for updates.
     */
    void listenToModel(const QSqlTableModel *model);

    /**
     * Ensure the given model updates when the given tables are updated.
     *
     * To prevent signal loops or other issues, the original model should
     * be connected to the DataManager using listenToModel.
     *
     * @param model The model to update. It will only listen to its own table.
     */
    void propagateToModel(QSqlTableModel *model);

    /**
     * Ensure the given model updates when the given tables are updated.
     *
     * To prevent signal loops or other issues, the original model should
     * be connected to the DataManager using listenToModel.
     *
     * @param model The model to update.
     * @param tables The tables to listen to.
     */
    void propagateToModel(QSqlTableModel *model, const QStringList &tables);

    /**
     * Ensure the given model updates when the given tables are updated.
     *
     * To prevent signal loops or other issues, the original model should
     * be connected to the DataManager using listenToModel.
     *
     * @tparam ModelType The type of the model.
     * @param model The model to update.
     * @param tables The tables to listen to.
     * @param updater Called when the model must update.
     */
    template<class ModelType>
    void propagateToModel(ModelType *model, QStringList tables, std::function<void(ModelType *)> updater);

    template<QSqlTableModelConcept ModelType, typename... Args>
    ModelType *makeModel(Args &&... args);
};
