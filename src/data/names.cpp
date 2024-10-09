//
// Created by niko on 25/09/24.
//

#include <KLocalizedString>
#include "names.h"
#include "database/schema.h"

NamesTableModel::NamesTableModel(QObject *parent) : QSqlRelationalTableModel(parent) {
    this->setTable(Schema::NamesTable);
    this->setRelation(NamesTableModel::ORIGIN,
                      QSqlRelation(
                              Schema::NameOriginsTable,
                              QStringLiteral("id"),
                              QStringLiteral("origins")
                      )
    );
    this->setJoinMode(QSqlRelationalTableModel::JoinMode::LeftJoin);

    // Set the correct headers.
    this->setHeaderData(ID, Qt::Horizontal, i18n("Id"));
    this->setHeaderData(PERSON_ID, Qt::Horizontal, i18n("Persoon-id"));
    this->setHeaderData(SORT, Qt::Horizontal, i18n("Volgorde"));
    this->setHeaderData(TITLES, Qt::Horizontal, i18n("Titels"));
    this->setHeaderData(GIVEN_NAMES, Qt::Horizontal, i18n("Voornamen"));
    this->setHeaderData(PREFIX, Qt::Horizontal, i18n("Voorvoegsels"));
    this->setHeaderData(SURNAME, Qt::Horizontal, i18n("Achternamen"));
    this->setHeaderData(ORIGIN, Qt::Horizontal, i18n("Oorsprong"));

    this->setSort(SORT, Qt::SortOrder::AscendingOrder);
}

NameOriginTableModel::NameOriginTableModel(QObject *parent) : QSqlTableModel(parent) {
    this->setTable(Schema::NameOriginsTable);

    this->setHeaderData(ID, Qt::Horizontal, i18n("Id"));
    this->setHeaderData(ORIGIN, Qt::Horizontal, i18n("Oorsprong"));
}

Qt::ItemFlags NameOriginTableModel::flags(const QModelIndex &index) const {
    auto flags = QSqlTableModel::flags(index);
    if (index.column() == NameOriginTableModel::ID) {
        flags = flags & ~Qt::ItemIsEditable;
    }
    return flags;
}
