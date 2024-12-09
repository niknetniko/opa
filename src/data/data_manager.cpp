/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "data_manager.h"

#include "data/names.h"
#include "dates/genealogical_date_proxy_model.h"
#include "event.h"
#include "family.h"
#include "person.h"
#include "utils/grouped_items_proxy_model.h"
#include "utils/model_utils.h"
#include "utils/multi_filter_proxy_model.h"

#include <KLocalizedString>
#include <KRearrangeColumnsProxyModel>
#include <QLibraryInfo>
#include <QSqlError>
#include <QSqlQuery>

DataManager* DataManager::instance = nullptr;

// NOLINTBEGIN(*-prefer-member-initializer)
DataManager::DataManager(QObject* parent) : QObject(parent) {
    baseNameOriginModel = makeModel<NameOriginTableModel>();
    baseNamesModel = makeModel<NamesTableModel>(baseNameOriginModel);
    baseEventRolesModel = makeModel<EventRolesModel>();
    baseEventRelationsModel = makeModel<EventRelationsModel>(baseEventRolesModel);
    baseEventTypesModel = makeModel<EventTypesModel>();
    baseEventsModel = makeModel<EventsModel>(baseEventTypesModel);
}
// NOLINTEND(*-prefer-member-initializer)

QSqlTableModel* DataManager::namesModel() const {
    return this->baseNamesModel;
}

QAbstractProxyModel* DataManager::namesModelForPerson(QObject* parent, const IntegerPrimaryKey personId) const {
    auto* baseProxy = new MultiFilterProxyModel(parent);
    baseProxy->setSourceModel(this->namesModel());
    baseProxy->addFilter(NamesTableModel::PERSON_ID, personId);

    auto* selectedColumnsModel = new KRearrangeColumnsProxyModel(parent);
    selectedColumnsModel->setSourceModel(baseProxy);
    selectedColumnsModel->setSourceColumns(
        {NamesTableModel::ID,
         NamesTableModel::SORT,
         NamesTableModel::TITLES,
         NamesTableModel::GIVEN_NAMES,
         NamesTableModel::PREFIX,
         NamesTableModel::SURNAME,
         NamesTableModel::ORIGIN}
    );

    auto* filterProxyModel = new QSortFilterProxyModel(parent);
    filterProxyModel->setSourceModel(selectedColumnsModel);

    return filterProxyModel;
}

QAbstractProxyModel* DataManager::singleNameModel(QObject* parent, const QVariant& nameId) const {
    auto* proxy = new MultiFilterProxyModel(parent);
    proxy->setSourceModel(this->namesModel());
    proxy->addFilter(NamesTableModel::ID, nameId);
    return proxy;
}

QAbstractProxyModel* DataManager::primaryNamesModel(QObject* parent) {
    auto query =
        QStringLiteral("SELECT people.id, names.titles, names.given_names, names.prefix, names.surname, "
                       "people.root "
                       "FROM people "
                       "JOIN names on people.id = names.person_id "
                       "WHERE names.sort = (SELECT MIN(n2.sort) FROM names AS n2 WHERE n2.person_id = people.id)");
    auto* baseModel = new QSqlQueryModel(parent);

    // These positions are hardcoded from the query above.
    baseModel->setQuery(query);
    baseModel->setHeaderData(0, Qt::Horizontal, i18n("Id"));
    baseModel->setHeaderData(1, Qt::Horizontal, i18n("Titles"));
    baseModel->setHeaderData(2, Qt::Horizontal, i18n("First names"));
    baseModel->setHeaderData(3, Qt::Horizontal, i18n("Prefixes"));
    baseModel->setHeaderData(4, Qt::Horizontal, i18n("Surnames"));
    baseModel->setHeaderData(5, Qt::Horizontal, i18n("Root"));

    // We want to add a column, where the name is produced based on other columns.
    auto* combinedModel = new DisplayNameProxyModel(parent);
    combinedModel->setSourceModel(baseModel);
    combinedModel->setColumns({
        .titles = 1,
        .givenNames = 2,
        .prefix = 3,
        .surname = 4,
    });

    // We want to re-arrange the columns and hide most of them.
    auto* rearrangedModel = new KRearrangeColumnsProxyModel(parent);
    rearrangedModel->setSourceModel(combinedModel);
    rearrangedModel->setSourceColumns({0, 6, 5});

    propagateToModel<QSqlQueryModel>(baseModel, {Schema::PeopleTable, Schema::NamesTable}, [query](auto* model) {
        model->setQuery(query);
    });

    return rearrangedModel;
}

QAbstractProxyModel* DataManager::personDetailsModel(QObject* parent, IntegerPrimaryKey personId) {
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

    auto* baseModel = new QSqlQueryModel(parent);
    baseModel->setQuery(std::move(query));
    baseModel->setHeaderData(PersonDetailModel::ID, Qt::Horizontal, i18n("Id"));
    baseModel->setHeaderData(PersonDetailModel::TITLES, Qt::Horizontal, i18n("Titels"));
    baseModel->setHeaderData(PersonDetailModel::GIVEN_NAMES, Qt::Horizontal, i18n("Voornamen"));
    baseModel->setHeaderData(PersonDetailModel::PREFIXES, Qt::Horizontal, i18n("Voorvoegsels"));
    baseModel->setHeaderData(PersonDetailModel::SURNAME, Qt::Horizontal, i18n("Achternaam"));
    baseModel->setHeaderData(PersonDetailModel::ROOT, Qt::Horizontal, i18n("Wortel"));
    baseModel->setHeaderData(PersonDetailModel::SEX, Qt::Horizontal, i18n("Geslacht"));

    // We want to add a column, where the name is produced based on other columns.
    auto* combinedModel = new DisplayNameProxyModel(parent);
    combinedModel->setSourceModel(baseModel);
    combinedModel->setColumns({
        PersonDetailModel::TITLES,
        PersonDetailModel::GIVEN_NAMES,
        PersonDetailModel::PREFIXES,
        PersonDetailModel::SURNAME,
    });

    propagateToModel<QSqlQueryModel>(
        baseModel,
        {Schema::PeopleTable, Schema::NamesTable},
        [rawQuery, personId](auto* model) {
            QSqlQuery newQuery;
            newQuery.prepare(rawQuery);
            newQuery.bindValue(QStringLiteral(":id"), personId);
            newQuery.exec();
            model->setQuery(std::move(newQuery));
        }
    );

    // Our QSqlQueryModel does not emit "data changed", since it is read-only.
    // This is, however, very annoying, so we do it anyway.
    connect(baseModel, &QAbstractItemModel::modelReset, [baseModel] {
        // TODO: should we properly pass an index here?
        Q_EMIT baseModel->dataChanged(baseModel->index(0, 0), baseModel->index(0, 0));
    });

    return combinedModel;
}

QSqlTableModel* DataManager::nameOriginsModel() const {
    return this->baseNameOriginModel;
}

QSqlTableModel* DataManager::eventRolesModel() const {
    return this->baseEventRolesModel;
}

QSqlTableModel* DataManager::eventTypesModel() const {
    return this->baseEventTypesModel;
}

QSqlTableModel* DataManager::eventRelationsModel() const {
    return this->baseEventRelationsModel;
}

QSqlTableModel* DataManager::eventsModel() const {
    return this->baseEventsModel;
}

QAbstractItemModel* DataManager::eventsModelWithDateSupport(QObject* parent) const {
    // TODO: this should happen in the base model instead.
    auto* model = eventsModel();
    auto* dateModel = new GenealogicalDateProxyModel(parent);
    dateModel->setSourceModel(model);
    dateModel->setDateColumn(EventsModel::DATE);
    return dateModel;
}

QAbstractProxyModel* DataManager::eventsModelForPerson(QObject* parent, IntegerPrimaryKey personId) {
    auto rawQuery = QStringLiteral("SELECT er.role, et.type, events.date, events.name, events.id, er.id "
                                   "FROM events "
                                   "LEFT JOIN event_types AS et ON events.type_id = et.id "
                                   "LEFT JOIN event_relations AS erel ON events.id = erel.event_id "
                                   "LEFT JOIN event_roles AS er ON er.id = erel.role_id "
                                   "WHERE erel.person_id = :id "
                                   "ORDER BY events.date ASC");
    QSqlQuery query;
    query.prepare(rawQuery);
    query.bindValue(QStringLiteral(":id"), personId);
    if (!query.exec()) {
        qWarning() << "Failed to execute query" << query.lastQuery();
        qWarning() << query.lastError();
        abort();
    }

    auto* baseModel = new QSqlQueryModel(parent);
    baseModel->setQuery(std::move(query));
    baseModel->setHeaderData(PersonEventsModel::ID, Qt::Horizontal, i18n("Id"));
    baseModel->setHeaderData(PersonEventsModel::DATE, Qt::Horizontal, i18n("Datum"));
    baseModel->setHeaderData(PersonEventsModel::NAME, Qt::Horizontal, i18n("Omschrijving"));
    baseModel->setHeaderData(PersonEventsModel::TYPE, Qt::Horizontal, i18n("Soort"));
    baseModel->setHeaderData(PersonEventsModel::ROLE, Qt::Horizontal, i18n("Rol"));
    baseModel->setHeaderData(PersonEventsModel::ROLE_ID, Qt::Horizontal, i18n("Rol-id"));

    // Connect the original model to changes.
    propagateToModel<QSqlQueryModel>(
        baseModel,
        {Schema::EventsTable, Schema::EventTypesTable, Schema::EventRolesTable, Schema::EventRelationsTable},
        [rawQuery, personId](auto* model) {
            QSqlQuery newQuery;
            newQuery.prepare(rawQuery);
            newQuery.bindValue(QStringLiteral(":id"), personId);
            newQuery.exec();
            model->setQuery(std::move(newQuery));
        }
    );

    auto* dateModel = new GenealogicalDateProxyModel(parent);
    dateModel->setSourceModel(baseModel);
    dateModel->setDateColumn(PersonEventsModel::DATE);

    auto* proxy = new GroupedItemsProxyModel(parent);
    proxy->setSourceModel(dateModel);
    proxy->setGroups({PersonEventsModel::ROLE});
    proxy->setGroupHeaderTitle(i18n("Rol"));

    // Hide the original role column.
    auto* hidden = new KRearrangeColumnsProxyModel(parent);
    hidden->setSourceModel(proxy);
    hidden->setSourceColumns({
        PersonEventsModel::ROLE,
        // These are all one further than we want.
        PersonEventsModel::TYPE + 1,
        PersonEventsModel::DATE + 1,
        PersonEventsModel::NAME + 1,
        PersonEventsModel::ID + 1,
        PersonEventsModel::ROLE_ID + 1,
    });

    return hidden;
}

QAbstractProxyModel* DataManager::singleEventModel(QObject* parent, const QVariant& eventId) const {
    auto* proxy = new MultiFilterProxyModel(parent);
    proxy->setSourceModel(this->eventsModelWithDateSupport(parent));
    proxy->addFilter(EventsModel::ID, eventId);
    return proxy;
}

QAbstractProxyModel* DataManager::singleEventRelationModel(
    QObject* parent, const QVariant& eventId, const QVariant& roleId, const QVariant& personId
) const {
    auto* proxy = new MultiFilterProxyModel(parent);
    proxy->setSourceModel(this->eventRelationsModel());
    proxy->addFilter(EventRelationsModel::EVENT_ID, eventId);
    proxy->addFilter(EventRelationsModel::ROLE_ID, roleId);
    proxy->addFilter(EventRelationsModel::PERSON_ID, personId);
    return proxy;
}

QAbstractProxyModel* DataManager::familyModelFor(QObject* parent, IntegerPrimaryKey person) {
    auto* proxyModel = new FamilyProxyModel(person, parent);

    propagateToModel<FamilyProxyModel>(
        proxyModel,
        {
            Schema::EventsTable,
            Schema::EventTypesTable,
            Schema::EventRolesTable,
            Schema::EventRelationsTable,
            Schema::NamesTable,
            Schema::PeopleTable,
        },
        [](auto* model) { model->resetAndLoadData(); }
    );

    auto* combinedModel = new DisplayNameProxyModel(parent);
    combinedModel->setSourceModel(proxyModel);
    combinedModel->setColumns({
        .titles = FamilyProxyModel::TITLES,
        .givenNames = FamilyProxyModel::GIVEN_NAMES,
        .prefix = FamilyProxyModel::PREFIX,
        .surname = FamilyProxyModel::SURNAME,
    });

    auto* columnModel = new KRearrangeColumnsProxyModel(parent);
    columnModel->setSourceModel(combinedModel);
    columnModel->setSourceColumns({
        FamilyProxyModel::EVENT_TYPE,
        FamilyProxyModel::DATE,
        FamilyProxyModel::PERSON_ID,
        combinedModel->columnCount() - 1,
        FamilyProxyModel::EVENT_ID,
    });

    auto* dateModel = new GenealogicalDateProxyModel(parent);
    dateModel->setSourceModel(columnModel);
    dateModel->setDateColumn(FamilyDisplayModel::DATE);

    return dateModel;
}

QAbstractProxyModel* DataManager::ancestorModelFor(QObject* parent, IntegerPrimaryKey person) {
    auto* ancestorQueryModel = new AncestorQueryModel(person, parent);
    propagateToModel<AncestorQueryModel>(
        ancestorQueryModel,
        {
            Schema::EventsTable,
            Schema::EventTypesTable,
            Schema::EventRolesTable,
            Schema::EventRelationsTable,
            Schema::NamesTable,
            Schema::PeopleTable,
        },
        [](auto* model) { model->resetAndLoadData(); }
    );

    auto* combinedModel = new DisplayNameProxyModel(parent);
    combinedModel->setSourceModel(ancestorQueryModel);
    combinedModel->setColumns({
        .titles = AncestorQueryModel::TITLES,
        .givenNames = AncestorQueryModel::GIVEN_NAMES,
        .prefix = AncestorQueryModel::PREFIX,
        .surname = AncestorQueryModel::SURNAME,
    });

    auto* columnModel = new KRearrangeColumnsProxyModel(parent);
    columnModel->setSourceModel(combinedModel);
    columnModel->setSourceColumns({
        AncestorQueryModel::CHILD_ID,
        AncestorQueryModel::FATHER_ID,
        AncestorQueryModel::MOTHER_ID,
        AncestorQueryModel::VISITED,
        AncestorQueryModel::LEVEL,
        combinedModel->columnCount() - 1,
    });

    return columnModel;
}

QAbstractProxyModel* DataManager::parentsModelFor(QObject* parent, IntegerPrimaryKey person) {
    auto* sourceModel = new ParentQueryModel(person, parent);
    propagateToModel<ParentQueryModel>(
        sourceModel,
        {
            Schema::EventsTable,
            Schema::EventTypesTable,
            Schema::EventRolesTable,
            Schema::EventRelationsTable,
            Schema::NamesTable,
            Schema::PeopleTable,
        },
        [](auto* model) { model->resetAndLoadData(); }
    );

    auto* combinedModel = new DisplayNameProxyModel(parent);
    combinedModel->setSourceModel(sourceModel);
    combinedModel->setColumns({
        .titles = ParentQueryModel::TITLES,
        .givenNames = ParentQueryModel::GIVEN_NAMES,
        .prefix = ParentQueryModel::PREFIX,
        .surname = ParentQueryModel::SURNAME,
    });

    auto* columnModel = new KRearrangeColumnsProxyModel(parent);
    columnModel->setSourceModel(combinedModel);
    columnModel->setSourceColumns({
        ParentQueryModel::ROLE,
        ParentQueryModel::PERSON_ID,
        combinedModel->columnCount() - 1,
    });

    return columnModel;
}

void DataManager::listenToModel(const QSqlTableModel* model) const {
    connect(model, &QSqlTableModel::dataChanged, this, &DataManager::onSourceModelChanged);
    connect(model, &QSqlTableModel::rowsInserted, this, &DataManager::onSourceModelChanged);
    connect(model, &QSqlTableModel::rowsRemoved, this, &DataManager::onSourceModelChanged);
    connect(model, &QSqlTableModel::modelReset, this, &DataManager::onSourceModelChanged);
}

void DataManager::onSourceModelChanged() {
    QObject* sender = QObject::sender();
    assert(sender != nullptr);

    auto* const sendingModel = qobject_cast<QSqlTableModel*>(sender);
    assert(sendingModel != nullptr);

    const auto metaMethod = sender->metaObject()->method(senderSignalIndex());

    // This is called as a slot by signals on the original models.
    // When this is called the first time, we will set the variable to true,
    // and propagate the signal to our own listeners.
    const auto senderName = QString::fromUtf8(sender->metaObject()->className());
    qDebug() << "This update is triggered by" << senderName << "for table" << sendingModel->tableName() << "by signal"
             << metaMethod.methodSignature();
    if (this->updatingFromDataManagerSource == nullptr) {
        // qDebug() << "   Not updating yet";
        updatingFromDataManagerSource = sender;
        Q_EMIT this->dataChanged(sendingModel->tableName());
        updatingFromDataManagerSource = nullptr;
    } else {
        // qDebug() << "   Already updating in DataManager, so ignoring this signal.";
        // We are already updating from another model, so do not propagate this.
        // Do nothing.
    }
}

void DataManager::propagateToModel(QSqlTableModel* model, const QStringList& tables) {
    this->propagateToModel<QSqlTableModel>(model, tables, [](QSqlTableModel* theModel) { theModel->select(); });
}

void DataManager::propagateToModel(QSqlTableModel* model) {
    this->propagateToModel(model, {model->tableName()});
}

void DataManager::initialize(QObject* parent) {
    assert(instance == nullptr);
    instance = new DataManager(parent);
}
void DataManager::reset() {
    assert(instance != nullptr);
    delete instance;
    instance = nullptr;
}

DataManager& DataManager::get() {
    return *instance;
}

template<QSqlTableModelConcept ModelType, typename... Args>
ModelType* DataManager::makeModel(Args&&... args) {
    auto* model = new ModelType(this, std::forward<Args>(args)...);
    model->setEditStrategy(QSqlTableModel::OnRowChange);
    if (!model->select()) {
        qCritical() << "Problem while getting data for model" << model->metaObject()->className();
        qWarning() << "Error was:" << model->lastError();
    }

    // Link the model to the DataManager.
    this->listenToModel(model);

    return model;
}

template<class ModelType>
void DataManager::propagateToModel(
    ModelType* model, QStringList tables, std::function<void(ModelType*)> updater // NOLINT(*-unnecessary-value-param)
) {
    connect(this, &DataManager::dataChanged, model, [model, tables, updater, this](const QString& table) {
        if (tables.contains(table) && updatingFromDataManagerSource != model) {
            updater(model);
        }
    });
}
