/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"
#include "utils/custom_sql_relational_model.h"

#include <QAbstractProxyModel>
#include <QObject>
#include <QSqlRelationalTableModel>

template<class M>
concept QSqlTableModelConcept = std::is_base_of_v<QSqlTableModel, M>;

/**
 * The centralized point to get models for data access from the database.
 *
 * All models returned by this class are "live": they will update if the underlying data changes.
 */
class DataManager : public QObject {
    Q_OBJECT

public:
    static void initialize(QObject* parent);
    static void reset();

    static DataManager& get();

    [[nodiscard]] QSqlTableModel* peopleModel() const;

    [[nodiscard]] QSqlTableModel* namesModel() const;

    [[nodiscard]] QSqlTableModel* nameOriginsModel() const;

    [[nodiscard]] QSqlTableModel* eventRolesModel() const;

    [[nodiscard]] QSqlTableModel* eventTypesModel() const;

    [[nodiscard]] QSqlTableModel* eventRelationsModel() const;

    [[nodiscard]] QSqlTableModel* eventsModel() const;

    [[nodiscard]] QAbstractItemModel* eventsModelWithDateSupport(QObject* parent) const;

    /**
     * Get a model representing all names for a single person.
     *
     * The returned model has columns as described in the \c PersonNamesModel namespace.
     *
     * @param parent The parent for the returned model.
     * @param personId The ID of the person to filter on.
     */
    [[nodiscard]] QAbstractProxyModel* namesModelForPerson(QObject* parent, IntegerPrimaryKey personId) const;

    /**
     * Model for a single name.
     *
     * @param parent The parent for the returned model.
     * @param nameId The ID of the name.
     */
    [[nodiscard]] QAbstractProxyModel* singleNameModel(QObject* parent, const QVariant& nameId) const;

    /**
     * Model for a single person.
     *
     * @param parent The parent for the returned model.
     * @param personId The ID of the person to filter on.
     */
    [[nodiscard]] QAbstractProxyModel* singlePersonModel(QObject* parent, IntegerPrimaryKey personId) const;

    /**
     * Model that returns the sexes in the database.
     *
     * @param parent The parent for the model
     */
    [[nodiscard]] QAbstractItemModel* sexesModel(QObject* parent);

    /**
     * Model showing only the primary names for each person.
     *
     * @param parent The parent of the model.
     */
    [[nodiscard]] QAbstractProxyModel* primaryNamesModel(QObject* parent);

    /**
     * Model for the details view of a person.
     */
    [[nodiscard]] QAbstractProxyModel* personDetailsModel(QObject* parent, IntegerPrimaryKey personId);

    /**
     * Model of events for one person (including all roles).
     *
     * The events are grouped under the role of the given person in the event.
     * See flatEventsModelForPerson for a flat model.
     *
     * See PersonEventsModel for an overview of the returned columns.
     */
    [[nodiscard]] QAbstractProxyModel* treeEventsModelForPerson(QObject* parent, IntegerPrimaryKey personId);

    /**
     * Model of events for one person (including all roles).
     *
     * The events are just a list of events.
     * See treeEventsModelForPerson for a tree model.
     *
     * See PersonEventsModel for an overview of the returned columns.
     */
    [[nodiscard]] QAbstractItemModel* flatEventsModelForPerson(QObject* parent, IntegerPrimaryKey personId);

    /**
     * Model of the birth event for a person.
     *
     * See PersonEventsModel for an overview of the returned columns.
     */
    [[nodiscard]] QAbstractItemModel* birthEventModelForPerson(QObject* parent, IntegerPrimaryKey personId);

    /**
     * Model for a single event.
     */
    [[nodiscard]] QAbstractProxyModel* singleEventModel(QObject* parent, const QVariant& eventId) const;

    /**
     * Model for a single event relation.
     */
    [[nodiscard]] QAbstractProxyModel* singleEventRelationModel(
        QObject* parent, const QVariant& eventId, const QVariant& roleId, const QVariant& personId
    ) const;

    /**
     * Model for event relation for a given person and event.
     */
    [[nodiscard]] QAbstractProxyModel*
    eventRelationModelByPersonAndEvent(QObject* parent, const QVariant& personId, const QVariant& eventId) const;

    /**
     * Model which returns all parent roles.
     *
     * Returns the same columns as EventRolesModel.
     */
    [[nodiscard]] QAbstractItemModel* parentEventRolesModel(QObject* parent) const;

    /**
     * Model returning all relationship event types.
     *
     * Returns the same columns as EventTypesModel.
     */
    [[nodiscard]] QAbstractItemModel* relationshipEventTypes(QObject* parent) const;

    /**
     * Tree model of all partners and all children of the given person.
     * The person itself is not included.
     * The children itself are arranged in a tree, with the partner being the root, and every child with that partner
     * being a child item.
     * Children without another parent are added under a synthetic "other" parent.
     *
     * See FamilyDisplayModel for an overview of the returned columns.
     */
    [[nodiscard]] QAbstractProxyModel* familyModelFor(QObject* parent, IntegerPrimaryKey person);

    /**
     * Model which will return all ancestors of the given person, including the person itself.
     *
     * See AncestorDisplayModel for an overview of the returned columns.
     */
    [[nodiscard]] QAbstractProxyModel* ancestorModelFor(QObject* parent, IntegerPrimaryKey person);

    /**
     * Model which returns all parents of a person.
     *
     * See DisplayParentModel for an overview of the returned columns.
     */
    [[nodiscard]] QAbstractProxyModel* parentsModelFor(QObject* parent, IntegerPrimaryKey person);

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
    static DataManager* instance;

    explicit DataManager(QObject* parent);

    /**
     * This is set to an address if the model updates are being sent from
     * the DataManager, otherwise it origines from the model itself.
     * Its main use is preventing signal loops, so if set to an address,
     * the updates will not propagate to the DataManager.
     */
    void* updatingFromDataManagerSource = nullptr;

    // Base models that allow editing.
    QSqlTableModel* basePeopleModel;
    CustomSqlRelationalModel* baseNamesModel;
    QSqlTableModel* baseNameOriginModel;
    QSqlTableModel* baseEventRolesModel;
    QSqlTableModel* baseEventTypesModel;
    CustomSqlRelationalModel* baseEventRelationsModel;
    CustomSqlRelationalModel* baseEventsModel;

    /**
     * Connect the given model to the DataManager. All updates in
     * the model will trigger the update of the dataChanged signal.
     *
     * @param model The model to listen to for updates.
     */
    void listenToModel(const QSqlTableModel* model) const;

    /**
     * Ensure the given model updates when the given tables are updated.
     *
     * To prevent signal loops or other issues, the original model should
     * be connected to the DataManager using listenToModel.
     *
     * @param model The model to update. It will only listen to its own table.
     */
    void propagateToModel(QSqlTableModel* model);

    /**
     * Ensure the given model updates when the given tables are updated.
     *
     * To prevent signal loops or other issues, the original model should
     * be connected to the DataManager using listenToModel.
     *
     * @param model The model to update.
     * @param tables The tables to listen to.
     */
    void propagateToModel(QSqlTableModel* model, const QStringList& tables);

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
    void propagateToModel(ModelType* model, QStringList tables, std::function<void(ModelType*)> updater);

    template<QSqlTableModelConcept ModelType, typename... Args>
    ModelType* makeModel(Args&&... args);
};
