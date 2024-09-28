//
// Created by niko on 25/09/24.
//

#include <KExtraColumnsProxyModel>
#include <KLocalizedString>
#include <QSqlQuery>
#include "data_manager.h"
#include "../names.h"
#include "utils/single_row_model.h"
#include "data/names.h"
#include "person.h"

DataManager *DataManager::instance = nullptr;

DataManager *DataManager::getInstance(QObject *parent) {
    if (instance == nullptr) {
        instance = new DataManager(parent);
    }
    return instance;
}

DataManager::DataManager(QObject *parent) : QObject(parent) {
    this->baseNamesModel = new NamesTableModel(this);
    this->baseNamesModel->select();

    connect(this->baseNamesModel, &QAbstractItemModel::dataChanged, this, &DataManager::onNamesTableChanged);
    connect(this->baseNamesModel, &QAbstractItemModel::rowsInserted, this, &DataManager::onNamesTableChanged);
    connect(this->baseNamesModel, &QAbstractItemModel::rowsRemoved, this, &DataManager::onNamesTableChanged);
}

QSqlTableModel *DataManager::namesModel() const {
    return this->baseNamesModel;
}

QAbstractProxyModel *DataManager::namesModelForPerson(QObject *parent, IntegerPrimaryKey personId) {
    auto *proxy = new CellFilteredProxyModel(parent, personId, NamesTableModel::PERSON_ID);
    proxy->setSourceModel(this->namesModel());
    return proxy;
}

void DataManager::onNamesTableChanged() {
    qDebug() << "Names table has changed....";
    Q_EMIT this->dataChanged(this->baseNamesModel->tableName());

    // Whe the data has changed, we need to reselect all names.
    // The reason being that the database itself can change rows with triggers.
    if (!this->baseNamesModel->isDirty()) {
        this->baseNamesModel->select();
    }
}

QAbstractProxyModel *DataManager::singleNameModel(QObject *parent, IntegerPrimaryKey nameId) {
    auto *proxy = new CellFilteredProxyModel(parent, nameId, NamesTableModel::ID);
    proxy->setSourceModel(this->namesModel());
    return proxy;
}

QAbstractProxyModel *DataManager::primaryNamesModel(QObject *parent) {
    auto query = QStringLiteral("SELECT people.id, names.titles, names.given_names, names.prefix, names.surname, people.root FROM people JOIN names on people.id = names.person_id WHERE (names.main = TRUE)");
    auto * baseModel = new QSqlQueryModel(parent);

    baseModel->setQuery(query);
    baseModel->setHeaderData(0, Qt::Horizontal, i18n("Id"));
    baseModel->setHeaderData(1, Qt::Horizontal, i18n("Titels"));
    baseModel->setHeaderData(2, Qt::Horizontal, i18n("Voornamen"));
    baseModel->setHeaderData(3, Qt::Horizontal, i18n("Voorvoegsels"));
    baseModel->setHeaderData(4, Qt::Horizontal, i18n("Achternaam"));
    baseModel->setHeaderData(5, Qt::Horizontal, i18n("Wortel"));

    // We want to add a column, where the name is produced based on other columns.
    auto combinedModel = new DisplayNameProxyModel(parent);
    combinedModel->setSourceModel(baseModel);

    // We want to re-arrange the columns and hide most of them.
    auto rearrangedModel = new KRearrangeColumnsProxyModel(parent);
    rearrangedModel->setSourceModel(combinedModel);
    rearrangedModel->setSourceColumns(QVector<int>() << 0 << 6 << 5);

    // Connect the original model to changes.
    connect(this, &DataManager::dataChanged, this, [=]( const QString &table ) {
        if (table == Schema::People::TableName || table == Schema::Names::TableName) {
            baseModel->setQuery(query);
        }
    });

    return rearrangedModel;
}
