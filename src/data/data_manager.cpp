//
// Created by niko on 25/09/24.
//

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
#include "utils/model_utils.h"

std::optional<DataManager> DataManager::instance = std::nullopt;

DataManager::DataManager(QObject *parent) : QObject(parent) {
    baseNameOriginModel = makeModel<NameOriginTableModel>();
    listenToModel(baseNameOriginModel);
    propagateToModel(baseNameOriginModel);

    baseNamesModel = makeModel<NamesTableModel>();
    listenToModel(baseNamesModel);
    propagateToModel(baseNamesModel, {baseNamesModel->tableName(), baseNameOriginModel->tableName()});

    // We also need to connect the relation model for the name origins from the base table.
    auto *nameOriginsRelationalModel = baseNamesModel->relationModel(NamesTableModel::ORIGIN);
    listenToModel(nameOriginsRelationalModel);
    propagateToModel(nameOriginsRelationalModel);

    baseEventRolesModel = makeModel<EventRolesModel>();
    listenToModel(baseEventRolesModel);
    propagateToModel(baseEventRolesModel);

    baseEventRelationsModel = makeModel<EventRelationsModel>();
    listenToModel(baseEventRelationsModel);
    propagateToModel(baseEventRelationsModel);


    baseEventTypesModel = makeModel<EventTypesModel>();
    listenToModel(baseEventTypesModel);
    propagateToModel(baseEventTypesModel);

    baseEventsModel = makeModel<EventsModel>();
    listenToModel(baseEventsModel);
    propagateToModel(baseEventsModel, {baseEventsModel->tableName(), baseEventTypesModel->tableName()});

    // We also need to connect the relation model for the event types from the base table.
    auto *eventTypesRelationalModel = baseEventsModel->relationModel(EventsModel::TYPE);
    listenToModel(eventTypesRelationalModel);
    propagateToModel(eventTypesRelationalModel);
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

    propagateToModel<QSqlQueryModel>(baseModel, {Schema::PeopleTable, Schema::NamesTable}, [query](auto* model) {
        model->setQuery(query);
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

    propagateToModel<QSqlQueryModel>(baseModel, {Schema::PeopleTable, Schema::NamesTable}, [rawQuery, personId](auto* model) {
        QSqlQuery newQuery;
        newQuery.prepare(rawQuery);
        newQuery.bindValue(QStringLiteral(":id"), personId);
        newQuery.exec();
        model->setQuery(std::move(newQuery));
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

QAbstractProxyModel *DataManager::eventsModelForPerson(QObject *parent, IntegerPrimaryKey personId) {
    auto rawQuery = QStringLiteral(
            "SELECT er.role, et.type, events.date, events.name, events.id "
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
    baseModel->setHeaderData(PersonEventsModel::TYPE, Qt::Horizontal, i18n("Soort"));
    baseModel->setHeaderData(PersonEventsModel::ROLE, Qt::Horizontal, i18n("Rol"));

    // Connect the original model to changes.
    propagateToModel<QSqlQueryModel>(baseModel, {Schema::EventsTable, Schema::EventTypesTable, Schema::EventRolesTable}, [rawQuery, personId](auto* model) {
        QSqlQuery newQuery;
        newQuery.prepare(rawQuery);
        newQuery.bindValue(QStringLiteral(":id"), personId);
        newQuery.exec();
        model->setQuery(std::move(newQuery));
    });

    auto *dateModel = new OpaDateModel(parent);
    dateModel->setSourceModel(baseModel);
    dateModel->setDateColumn(PersonEventsModel::DATE);

    GroupedItemsProxyModel *proxy = new GroupedItemsProxyModel(parent);
    proxy->setSourceModel(dateModel);
    proxy->setGroups({PersonEventsModel::ROLE});
    proxy->setGroupHeaderTitle(i18n("Rol"));

    // Hide the original role column.
    auto *hidden = new KRearrangeColumnsProxyModel(parent);
    hidden->setSourceModel(proxy);
    hidden->setSourceColumns({
                                     PersonEventsModel::ROLE,
                                     // These are all one further than we want.
                                     PersonEventsModel::TYPE + 1,
                                     PersonEventsModel::DATE + 1,
                                     PersonEventsModel::NAME + 1,
                                     PersonEventsModel::ID + 1
                             });

    auto sortable = new QSortFilterProxyModel(parent);
    sortable->setSourceModel(hidden);

    return sortable;
}

void DataManager::listenToModel(QSqlTableModel *model) {
    connect(model, &QAbstractItemModel::dataChanged, this, &DataManager::onSourceModelChanged);
    connect(model, &QAbstractItemModel::rowsInserted, this, &DataManager::onSourceModelChanged);
    connect(model, &QAbstractItemModel::rowsRemoved, this, &DataManager::onSourceModelChanged);
    connect(model, &QAbstractItemModel::modelReset, this, &DataManager::onSourceModelChanged);
}

void DataManager::onSourceModelChanged() {
    QObject *sender = QObject::sender();
    assert(sender != nullptr);

    auto sendingModel = qobject_cast<QSqlTableModel *>(sender);
    assert(sendingModel != nullptr);

    // This is called as a slot by signals on the original models.
    // When this is called the first time, we will set the variable to true,
    // and propagate the signal to our own listeners.
    QString senderName = QString::fromUtf8(sender->metaObject()->className());
    qDebug() << "This update is triggered by" << senderName << "for table" << sendingModel->tableName();
    if (this->updatingFromDataManagerSource == nullptr) {
        qDebug() << "Not updating yet";
        updatingFromDataManagerSource = sender;
        Q_EMIT this->dataChanged(sendingModel->tableName());
        updatingFromDataManagerSource = nullptr;
    } else {
        qDebug() << "Already updating in DataManager, so ignoring this signal." << senderName;
        // We are already updating from another model, so do not propagate this.
        // Do nothing.
    }
}

void DataManager::propagateToModel(QSqlTableModel *model, QStringList tables) {
    this->propagateToModel<QSqlTableModel>(model, tables, [](QSqlTableModel* theModel) {
        theModel->select();
    });
}

void DataManager::propagateToModel(QSqlTableModel *model) {
    this->propagateToModel(model, {model->tableName()});
}

void DataManager::initialize(QObject *parent) {
    assert(!instance);
    instance.emplace(parent);
}

DataManager &DataManager::get() {
    assert(instance);
    return instance.value();
}

template<QSqlTableModelConcept ModelType>
ModelType *DataManager::makeModel() {
    auto* model = new ModelType(this);
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    if (!model->select()) {
        qWarning() << "Problem while getting data for model" << model->metaObject()->className();
        auto lastError = model->lastError();
        auto errorText = lastError.text();
        qWarning() << "Error was:" << errorText;
        qDebug() << "Raw error: " << lastError;
    }
    return model;
}

template<class ModelType>
void DataManager::propagateToModel(ModelType *model, QStringList tables, std::function<void(ModelType *)> updater) {
    auto name = model->metaObject()->className();
    connect(this, &DataManager::dataChanged, model, [model, tables, updater, name, this](QString table){
        qDebug() << "Propagating update of table" << table << "to model " << name;
        if (tables.contains(table)) {
            if (updatingFromDataManagerSource != model) {
                qDebug() << "   Model is interested, triggering refresh of model.";
                updater(model);
            } else {
                qDebug() << "   Model is interested, but is the source, skipping.";
            }
        } else {
            qDebug() << "   Model is not interested, skipping.";
        }
    });
}
