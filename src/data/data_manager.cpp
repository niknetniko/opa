//
// Created by niko on 25/09/24.
//

#include <KExtraColumnsProxyModel>
#include <KLocalizedString>
#include <QSqlQuery>
#include <QSqlError>
#include <QLibraryInfo>

#include "data_manager.h"
#include "names/names_overview_view.h"
#include "utils/single_row_model.h"
#include "data/names.h"
#include "person.h"
#include "event.h"
#include "utils/grouped_items_proxy_model.h"

DataManager *DataManager::instance = nullptr;

DataManager *DataManager::getInstance(QObject *parent) {
    if (instance == nullptr) {
        instance = new DataManager(parent);
    }
    return instance;
}

DataManager::DataManager(QObject *parent) : QObject(parent) {
    this->baseNamesModel = new NamesTableModel(this);
    this->baseNamesModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    this->baseNameOriginModel = new NameOriginTableModel(this);
    this->baseNameOriginModel->setEditStrategy(QSqlTableModel::OnFieldChange);

    baseNamesModel->select();
    // TODO: reduce the number of unnecessary signals here
    // Connect the base model.
    connect(baseNamesModel, &QAbstractItemModel::dataChanged, this, &DataManager::onSqlModelChanged);
    connect(baseNamesModel, &QAbstractItemModel::rowsInserted, this, &DataManager::onSqlModelChanged);
    connect(baseNamesModel, &QAbstractItemModel::rowsRemoved, this, &DataManager::onSqlModelChanged);
    connect(baseNamesModel, &QAbstractItemModel::modelReset, this, &DataManager::onSqlModelChanged);

    // Connect the other model.
    auto *baseNamesRelationModel = baseNamesModel->relationModel(NamesTableModel::ORIGIN);
    // This updater is triggered when the base origin model is updated.
    auto relationalListener = [this]() {
        // If we are already updating the origin table, e.g. from the other model,
        // do not propagate this update any further to prevent loops.
        if (updatingNameOrigin) {
            qDebug() << "Already updating so skipping update of base model.";
            return;
        }

        qDebug() << "Doing base model.";

        updatingNameOrigin = true;
        // Update the other model.
        baseNameOriginModel->select();
        Q_EMIT this->dataChanged(this->baseNameOriginModel->tableName());
        // Done.
        updatingNameOrigin = false;
    };
    connect(baseNamesRelationModel, &QAbstractItemModel::dataChanged, this, relationalListener);
    connect(baseNamesRelationModel, &QAbstractItemModel::rowsInserted, this, relationalListener);
    connect(baseNamesRelationModel, &QAbstractItemModel::rowsRemoved, this, relationalListener);
    connect(baseNamesRelationModel, &QAbstractItemModel::modelReset, this, relationalListener);
//    connect(this, &DataManager::dataChanged, baseNameOriginModel, baseModelUpdater);

    baseNameOriginModel->select();
    auto baseModelListener = [this]() {
        // If we are already updating the origin table, e.g. from the other model,
        // do not propagate this update any further to prevent loops.
        if (updatingNameOrigin) {
            qDebug() << "Already updating so skipping update of relational model.";
            return;
        }

        qDebug() << "Doing relational model.";

        updatingNameOrigin = true;
        // Update the other model.
        baseNamesModel->select();
        Q_EMIT this->dataChanged(this->baseNameOriginModel->tableName());
        // Done.
        updatingNameOrigin = false;
    };
    connect(baseNameOriginModel, &QAbstractItemModel::dataChanged, this, baseModelListener);
    connect(baseNameOriginModel, &QAbstractItemModel::rowsInserted, this, baseModelListener);
    connect(baseNameOriginModel, &QAbstractItemModel::rowsRemoved, this, baseModelListener);
    connect(baseNameOriginModel, &QAbstractItemModel::modelReset, this, baseModelListener);

    this->baseEventRolesModel = new EventRolesModel(this);
    this->baseEventRolesModel->select();
    connect(baseEventRolesModel, &QAbstractItemModel::dataChanged, this, &DataManager::onSqlModelChanged);
    connect(baseEventRolesModel, &QAbstractItemModel::rowsInserted, this, &DataManager::onSqlModelChanged);
    connect(baseEventRolesModel, &QAbstractItemModel::rowsRemoved, this, &DataManager::onSqlModelChanged);
    connect(baseEventRolesModel, &QAbstractItemModel::modelReset, this, &DataManager::onSqlModelChanged);

    this->baseEventRelationsModel = new EventRelationsModel(this);
    this->baseEventRelationsModel->select();
    connect(baseEventRelationsModel, &QAbstractItemModel::dataChanged, this, &DataManager::onSqlModelChanged);
    connect(baseEventRelationsModel, &QAbstractItemModel::rowsInserted, this, &DataManager::onSqlModelChanged);
    connect(baseEventRelationsModel, &QAbstractItemModel::rowsRemoved, this, &DataManager::onSqlModelChanged);
    connect(baseEventRelationsModel, &QAbstractItemModel::modelReset, this, &DataManager::onSqlModelChanged);

    this->baseEventTypesModel = new EventTypesModel(this);
    baseEventTypesModel->select();
    this->baseEventsModel = new EventsModel(this);
    baseEventsModel->select();

    // Connect the other model.
    auto *baseEventTypeRelationalModel = baseEventsModel->relationModel(EventsModel::TYPE);
    // This updater is triggered when the base origin model is updated.
    auto eventTypeRelationalListener = [this]() {
        // If we are already updating the origin table, e.g. from the other model,
        // do not propagate this update any further to prevent loops.
        if (updatingEventType) {
            qDebug() << "Already updating so skipping update of base model.";
            return;
        }

        qDebug() << "Doing base model.";

        updatingEventType = true;
        // Update the other model.
        baseEventTypesModel->select();
        Q_EMIT this->dataChanged(this->baseEventTypesModel->tableName());
        // Done.
        updatingEventType = false;
    };
    connect(baseEventTypeRelationalModel, &QAbstractItemModel::dataChanged, this, eventTypeRelationalListener);
    connect(baseEventTypeRelationalModel, &QAbstractItemModel::rowsInserted, this, eventTypeRelationalListener);
    connect(baseEventTypeRelationalModel, &QAbstractItemModel::rowsRemoved, this, eventTypeRelationalListener);
    connect(baseEventTypeRelationalModel, &QAbstractItemModel::modelReset, this, eventTypeRelationalListener);

    auto baseEventModelListener = [this]() {
        // If we are already updating the origin table, e.g. from the other model,
        // do not propagate this update any further to prevent loops.
        if (updatingEventType) {
            qDebug() << "Already updating so skipping update of relational model.";
            return;
        }

        qDebug() << "Doing relational model.";

        updatingEventType = true;
        // Update the other model.
        baseEventsModel->select();
        Q_EMIT this->dataChanged(this->baseEventTypesModel->tableName());
        // Done.
        updatingEventType = false;
    };
    connect(baseEventTypesModel, &QAbstractItemModel::dataChanged, this, baseEventModelListener);
    connect(baseEventTypesModel, &QAbstractItemModel::rowsInserted, this, baseEventModelListener);
    connect(baseEventTypesModel, &QAbstractItemModel::rowsRemoved, this, baseEventModelListener);
    connect(baseEventTypesModel, &QAbstractItemModel::modelReset, this, baseEventModelListener);
}

QSqlTableModel *DataManager::namesModel() const {
    return this->baseNamesModel;
}

QAbstractProxyModel *DataManager::namesModelForPerson(QObject *parent, IntegerPrimaryKey personId) {
    auto *proxy = new CellFilteredProxyModel(parent, personId, NamesTableModel::PERSON_ID);
    proxy->setSourceModel(this->namesModel());
    return proxy;
}

QAbstractProxyModel *DataManager::singleNameModel(QObject *parent, IntegerPrimaryKey nameId) {
    qDebug() << "Creating single model where the ID is " << nameId << " on column " << NamesTableModel::ID;
    auto *proxy = new CellFilteredProxyModel(parent, nameId, NamesTableModel::ID);
    proxy->setSourceModel(this->namesModel());
    return proxy;
}

QAbstractProxyModel *DataManager::primaryNamesModel(QObject *parent) {
    auto query = QStringLiteral(
            "SELECT people.id, names.titles, names.given_names, names.prefix, names.surname, people.root "
            "FROM people "
            "JOIN names on people.id = names.person_id "
            "WHERE names.sort = (SELECT MIN(n2.sort) FROM names AS n2 WHERE n2.person_id = people.id)"
    );
    auto *baseModel = new QSqlQueryModel(parent);

    // These positions are hardcoded from the query above.
    baseModel->setQuery(query);
    baseModel->setHeaderData(0, Qt::Horizontal, i18n("Id"));
    baseModel->setHeaderData(1, Qt::Horizontal, i18n("Titels"));
    baseModel->setHeaderData(2, Qt::Horizontal, i18n("Voornamen"));
    baseModel->setHeaderData(3, Qt::Horizontal, i18n("Voorvoegsels"));
    baseModel->setHeaderData(4, Qt::Horizontal, i18n("Achternaam"));
    baseModel->setHeaderData(5, Qt::Horizontal, i18n("Wortel"));

    // We want to add a column, where the name is produced based on other columns.
    auto *combinedModel = new DisplayNameProxyModel(parent);
    combinedModel->setSourceModel(baseModel);

    // We want to re-arrange the columns and hide most of them.
    auto *rearrangedModel = new KRearrangeColumnsProxyModel(parent);
    rearrangedModel->setSourceModel(combinedModel);
    rearrangedModel->setSourceColumns(QVector<int>() << 0 << 6 << 5);

    // Connect the original model to changes.
    connect(this, &DataManager::dataChanged, this, [baseModel, query](const QString &table) {
        if (table == Schema::PeopleTable || table == Schema::NamesTable) {
            baseModel->setQuery(query);
        }
    });

    return rearrangedModel;
}

QAbstractProxyModel *DataManager::personDetailsModel(QObject *parent, IntegerPrimaryKey personId) {
    auto rawQuery = QStringLiteral(
            "SELECT people.id, names.titles, names.given_names, names.prefix, names.surname, people.root, people.sex "
            "FROM people "
            "JOIN names on people.id = names.person_id "
            "WHERE names.sort = (SELECT MIN(n2.sort) FROM names AS n2 WHERE n2.person_id = people.id) AND people.id = :id"
    );
    QSqlQuery query;
    query.prepare(rawQuery);
    query.bindValue(QStringLiteral(":id"), personId);
    query.exec();
    qDebug() << "DETAILS OK";

    auto *baseModel = new QSqlQueryModel(parent);
    baseModel->setQuery(std::move(query));
    baseModel->setHeaderData(PersonDetailModel::ID, Qt::Horizontal, i18n("Id"));
    baseModel->setHeaderData(PersonDetailModel::TITLES, Qt::Horizontal, i18n("Titels"));
    baseModel->setHeaderData(PersonDetailModel::GIVEN_NAMES, Qt::Horizontal, i18n("Voornamen"));
    baseModel->setHeaderData(PersonDetailModel::PREFIXES, Qt::Horizontal, i18n("Voorvoegsels"));
    baseModel->setHeaderData(PersonDetailModel::SURNAME, Qt::Horizontal, i18n("Achternaam"));
    baseModel->setHeaderData(PersonDetailModel::ROOT, Qt::Horizontal, i18n("Wortel"));
    baseModel->setHeaderData(PersonDetailModel::SEX, Qt::Horizontal, i18n("Geslacht"));

    // We want to add a column, where the name is produced based on other columns.
    auto *combinedModel = new DisplayNameProxyModel(parent);
    combinedModel->setSourceModel(baseModel);

    // Connect the original model to changes.
    connect(this, &DataManager::dataChanged, baseModel, [=](const QString &table) {
        if (table == Schema::PeopleTable || table == Schema::NamesTable) {
            QSqlQuery newQuery;
            newQuery.prepare(rawQuery);
            newQuery.bindValue(QStringLiteral(":id"), personId);
            newQuery.exec();
            baseModel->setQuery(std::move(newQuery));
        }
    });

    // Our QSqlQueryModel does not emit "data changed", since it is read-only.
    // This is, however, very annoying, so we do it anyway.
    connect(baseModel, &QAbstractItemModel::modelReset, [baseModel]() {
        // TODO: should we properly pass an index here?
        Q_EMIT baseModel->dataChanged(baseModel->index(0, 0), baseModel->index(0, 0));
    });

    return combinedModel;
}

QSqlTableModel *DataManager::nameOriginsModel() const {
    return this->baseNameOriginModel;
}

QSqlTableModel *DataManager::eventRolesModel() const {
    return this->baseEventRolesModel;
}

QSqlTableModel *DataManager::eventTypesModel() const {
    return this->baseEventTypesModel;
}

QSqlTableModel *DataManager::eventRelationsModel() const {
    return this->baseEventRelationsModel;
}

QSqlTableModel *DataManager::eventsModel() {
    return this->baseEventsModel;
}

void DataManager::onSqlModelChanged() {
    QObject *sender = QObject::sender();
    if (sender == nullptr) {
        return;
    }

    auto sendingModel = qobject_cast<QSqlTableModel *>(sender);
    assert(sendingModel != nullptr);

    Q_EMIT this->dataChanged(sendingModel->tableName());
}

QAbstractProxyModel *DataManager::eventsModelForPerson(QObject *parent, IntegerPrimaryKey personId) {
    auto rawQuery = QStringLiteral(
            "SELECT er.role, events.id, events.date, events.name, et.type "
            "FROM events "
            "LEFT JOIN event_types AS et ON events.type_id = et.id "
            "LEFT JOIN event_relations AS erel ON events.id = erel.event_id "
            "LEFT JOIN event_roles AS er ON er.id = erel.role_id "
            "WHERE erel.person_id = :id "
            "ORDER BY events.date ASC"
    );
    QSqlQuery query;
    query.prepare(rawQuery);
    query.bindValue(QStringLiteral(":id"), personId);
    query.exec();

    auto *baseModel = new QSqlQueryModel(parent);
    baseModel->setQuery(std::move(query));
    baseModel->setHeaderData(PersonEventsModel::ID, Qt::Horizontal, i18n("Id"));
    baseModel->setHeaderData(PersonEventsModel::DATE, Qt::Horizontal, i18n("Datum"));
    baseModel->setHeaderData(PersonEventsModel::NAME, Qt::Horizontal, i18n("Omschrijving"));
    baseModel->setHeaderData(PersonEventsModel::TYPE, Qt::Horizontal, i18n("Type"));
    baseModel->setHeaderData(PersonEventsModel::ROLE, Qt::Horizontal, i18n("Rol"));

    // Connect the original model to changes.
    connect(this, &DataManager::dataChanged, baseModel, [=](const QString &table) {
        if (table == Schema::EventsTable || table == Schema::EventTypesTable || table == Schema::EventRolesTable ||
            table == Schema::EventRelationsTable) {
            QSqlQuery newQuery;
            newQuery.prepare(rawQuery);
            newQuery.bindValue(QStringLiteral(":id"), personId);
            newQuery.exec();
            baseModel->setQuery(std::move(newQuery));
        }
    });

    GroupedItemsProxyModel *proxy = new GroupedItemsProxyModel(parent);
    proxy->setSourceModel(baseModel);
    proxy->setGroups({PersonEventsModel::ROLE});
    proxy->setGroupHeaderTitle(i18n("Rol"));

    // Hide the original role column.
    auto *hidden = new KRearrangeColumnsProxyModel(parent);
    hidden->setSourceModel(proxy);
    hidden->setSourceColumns({
                                     PersonEventsModel::ROLE,
                                     // These are all one further than we want.
                                     PersonEventsModel::ID + 1,
                                     PersonEventsModel::DATE + 1,
                                     PersonEventsModel::NAME + 1,
                                     PersonEventsModel::TYPE + 1
                             });

    auto sortable = new QSortFilterProxyModel(parent);
    sortable->setSourceModel(hidden);

    return sortable;
}
