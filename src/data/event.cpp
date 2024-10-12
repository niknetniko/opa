#include <QString>
#include <KLocalizedString>

#include "event.h"
#include "database/schema.h"
#include "utils/opa_date.h"
#include "utils/model_utils.h"

EventRolesModel::EventRolesModel(QObject *parent) : QSqlTableModel(parent) {
    this->setTable(Schema::EventRolesTable);

    this->setHeaderData(ID, Qt::Horizontal, i18n("Id"));
    this->setHeaderData(ROLE, Qt::Horizontal, i18n("Rol"));
}

EventTypesModel::EventTypesModel(QObject *parent) : QSqlTableModel(parent) {
    this->setTable(Schema::EventTypesTable);

    this->setHeaderData(ID, Qt::Horizontal, i18n("Id"));
    this->setHeaderData(TYPE, Qt::Horizontal, i18n("Soort"));
}

EventRelationsModel::EventRelationsModel(QObject *parent) : QSqlTableModel(parent) {
    this->setTable(Schema::EventRelationsTable);

    this->setHeaderData(EVENT_ID, Qt::Horizontal, i18n("Gebeurtenis-id"));
    this->setHeaderData(PERSON_ID, Qt::Horizontal, i18n("Persoon-id"));
    this->setHeaderData(ROLE_ID, Qt::Horizontal, i18n("Rol-id"));
}

EventsModel::EventsModel(QObject *parent) : QSqlRelationalTableModel(parent) {
    this->setTable(Schema::EventsTable);
    this->setRelation(EventsModel::TYPE,
                      QSqlRelation(
                              Schema::EventTypesTable,
                              QStringLiteral("id"),
                              QStringLiteral("type")
                      )
    );
    this->setJoinMode(QSqlRelationalTableModel::JoinMode::LeftJoin);

    this->setHeaderData(ID, Qt::Horizontal, i18n("Id"));
    this->setHeaderData(TYPE, Qt::Horizontal, i18n("Soort"));
    this->setHeaderData(DATE, Qt::Horizontal, i18n("Datum"));
    this->setHeaderData(NAME, Qt::Horizontal, i18n("Naam"));
}

QVariant EventsModel::data(const QModelIndex &item, int role) const {
    if (item.column() == DATE) {
        auto rawData = QSqlRelationalTableModel::data(item, role).toString();
        auto model = OpaDate::fromDatabaseRepresentation(rawData);
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return model.toDisplayText();
        } else if (role == ModelRole) {
            return QVariant::fromValue(model);
        }
    }

    return QSqlRelationalTableModel::data(item, role);
}

bool EventsModel::setData(const QModelIndex &item, const QVariant &value, int role) {
    if (item.column() == DATE && (role == Qt::DisplayRole || role == Qt::EditRole || role == ModelRole)) {
        OpaDate model;
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            model = OpaDate::fromDisplayText(value.toString());
        } else {
            assert(role == ModelRole);
            model = value.value<OpaDate>();
        }
        auto databaseRepresentation = model.toDatabaseRepresentation();
        return QSqlRelationalTableModel::setData(item, databaseRepresentation, role);
    }

    return QSqlRelationalTableModel::setData(item, value, role);
}
