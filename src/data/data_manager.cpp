/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "data_manager.h"

#include "../core/data_event_broker.h"
#include "source.h"
#include "utils/grouping_proxy_model.h"
#include "utils/multi_filter_proxy_model.h"
#include "utils/tree_proxy_model.h"

#include <KLocalizedString>
#include <QLibraryInfo>
#include <QSqlError>
#include <QSqlQuery>

DataManager* DataManager::instance = nullptr;

// NOLINTBEGIN(*-prefer-member-initializer)
DataManager::DataManager(QObject* parent) : QObject(parent) {
    baseSourcesModel = makeModel<SourcesTableModel>();
}
// NOLINTEND(*-prefer-member-initializer)

QAbstractProxyModel* DataManager::singleSourceModel(QObject* parent, const QVariant& sourceId) const {
    auto* proxy = new MultiFilterProxyModel(parent);
    proxy->setSourceModel(this->sourcesModel());
    proxy->addFilter(SourcesTableModel::ID, sourceId);
    return proxy;
}


QSqlTableModel* DataManager::sourcesModel() const {
    return this->baseSourcesModel;
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
