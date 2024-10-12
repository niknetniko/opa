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
