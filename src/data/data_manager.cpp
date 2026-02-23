/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "data_manager.h"

#include "../core/data_event_broker.h"
#include "data/names.h"
#include "dates/genealogical_date_proxy_model.h"
#include "event.h"
#include "family.h"
#include "source.h"
#include "utils/grouping_proxy_model.h"
#include "utils/multi_filter_proxy_model.h"
#include "utils/tree_proxy_model.h"

#include <KLocalizedString>
#include <KRearrangeColumnsProxyModel>
#include <QLibraryInfo>
#include <QSqlError>
#include <QSqlQuery>

DataManager* DataManager::instance = nullptr;

// NOLINTBEGIN(*-prefer-member-initializer)
DataManager::DataManager(QObject* parent) : QObject(parent) {
    baseEventRolesModel = makeModel<EventRolesModel>();
    baseEventRelationsModel = makeModel<EventRelationsModel>(baseEventRolesModel);
    baseEventTypesModel = makeModel<EventTypesModel>();
    baseEventsModel = makeModel<EventsModel>(baseEventTypesModel);
    baseSourcesModel = makeModel<SourcesTableModel>();
}
// NOLINTEND(*-prefer-member-initializer)

QAbstractProxyModel* DataManager::singleSourceModel(QObject* parent, const QVariant& sourceId) const {
    auto* proxy = new MultiFilterProxyModel(parent);
    proxy->setSourceModel(this->sourcesModel());
    proxy->addFilter(SourcesTableModel::ID, sourceId);
    return proxy;
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
    auto* model = eventsModel();
    auto* dateModel = new GenealogicalDateProxyModel(parent);
    dateModel->setSourceModel(model);
    dateModel->setDateColumn(EventsModel::DATE);
    return dateModel;
}

QSqlTableModel* DataManager::sourcesModel() const {
    return this->baseSourcesModel;
}

QAbstractProxyModel* DataManager::eventRelationModelByPersonAndEvent(
    QObject* parent, const QVariant& personId, const QVariant& eventId
) const {
    auto* proxy = new MultiFilterProxyModel(parent);
    proxy->setSourceModel(this->eventRelationsModel());
    proxy->addFilter(EventRelationsModel::EVENT_ID, eventId);
    proxy->addFilter(EventRelationsModel::PERSON_ID, personId);
    return proxy;
}

QAbstractItemModel* DataManager::parentEventRolesModel(QObject* parent) const {
    return new ParentEventRolesModel(parent);
}

QAbstractItemModel* DataManager::relationshipEventTypes(QObject* parent) const {
    return new RelationshipEventTypesModel(parent);
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

QAbstractItemModel* DataManager::sourcesTreeModel(QObject* parent) const {
    auto* treeModel = new TreeProxyModel(parent);
    treeModel->setSourceModel(this->sourcesModel());
    debugPrintModel(this->sourcesModel());
    treeModel->setIdColumn(SourcesTableModel::ID);
    treeModel->setParentIdColumn(SourcesTableModel::PARENT_ID);
    // Print when the model changes
    connect(treeModel, &TreeProxyModel::dataChanged, this, []() {
        qDebug() << "Source tree model changed";
    });

    return treeModel;
}

QAbstractItemModel* DataManager::sourceTypeModel(QObject* parent) {
    auto query = QStringLiteral("SELECT DISTINCT type FROM sources");
    auto* baseModel = new QSqlQueryModel(parent);
    baseModel->setQuery(query);
    propagateToModel<QSqlQueryModel>(baseModel, {Schema::SourcesTable}, [query](auto* model) {
        model->setQuery(query);
    });
    return baseModel;
}

void DataManager::listenToModel(const QSqlTableModel* model) const {
    connect(model, &QSqlTableModel::dataChanged, this, &DataManager::onSourceModelChanged);
    connect(model, &QSqlTableModel::rowsInserted, this, &DataManager::onSourceModelChanged);
    connect(model, &QSqlTableModel::rowsRemoved, this, &DataManager::onSourceModelChanged);
    connect(model, &QSqlTableModel::modelReset, this, &DataManager::onSourceModelChanged);
}

void DataManager::onSourceModelChanged() {
    QObject* sender = QObject::sender();
    Q_ASSERT(sender != nullptr);

    auto* const sendingModel = qobject_cast<QSqlTableModel*>(sender);
    Q_ASSERT(sendingModel != nullptr);

    const auto metaMethod = sender->metaObject()->method(senderSignalIndex());

    // This is called as a slot by signals on the original models.
    // When this is called the first time, we will set the variable to true,
    // and propagate the signal to our own listeners.
    const auto senderName = QString::fromUtf8(sender->metaObject()->className());
    qDebug() << "This update is triggered by" << senderName << "for table" << sendingModel->tableName() << "by signal"
             << metaMethod.methodSignature();
    if (this->updatingFromDataManagerSource == nullptr) {
        qDebug() << "   Not updating yet";
        updatingFromDataManagerSource = sender;
        Q_EMIT this->dataChanged(sendingModel->tableName());
        updatingFromDataManagerSource = nullptr;
    } else {
        qDebug() << "   Already updating in DataManager, so ignoring this signal.";
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
    Q_ASSERT(instance == nullptr);
    instance = new DataManager(parent);
}
void DataManager::reset() {
    Q_ASSERT(instance != nullptr);
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
    ModelType* model,
    const QStringList& tables,
    std::function<void(ModelType*)> updater // NOLINT(*-unnecessary-value-param)
) {
    connect(this, &DataManager::dataChanged, model, [model, tables, updater, this](const QString& table) {
        if (tables.contains(table) && updatingFromDataManagerSource != model) {
            updater(model);
        }
    });
}
