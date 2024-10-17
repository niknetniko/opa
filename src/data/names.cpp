//
// Created by niko on 25/09/24.
//

#include <KLocalizedString>
#include "names.h"
#include "database/schema.h"

NamesTableModel::NamesTableModel(QObject *parent, QSqlTableModel* originsModel) : CustomSqlRelationalModel(parent) {
    CustomSqlRelationalModel::setTable(Schema::NamesTable);
    setRelation(ORIGIN_ID, originsModel, NameOriginTableModel::ORIGIN, NameOriginTableModel::ID);

    // Set the correct headers.
    CustomSqlRelationalModel::setHeaderData(ID, Qt::Horizontal, i18n("Id"));
    CustomSqlRelationalModel::setHeaderData(PERSON_ID, Qt::Horizontal, i18n("Persoon-id"));
    CustomSqlRelationalModel::setHeaderData(SORT, Qt::Horizontal, i18n("Volgorde"));
    CustomSqlRelationalModel::setHeaderData(TITLES, Qt::Horizontal, i18n("Titels"));
    CustomSqlRelationalModel::setHeaderData(GIVEN_NAMES, Qt::Horizontal, i18n("Voornamen"));
    CustomSqlRelationalModel::setHeaderData(PREFIX, Qt::Horizontal, i18n("Voorvoegsels"));
    CustomSqlRelationalModel::setHeaderData(SURNAME, Qt::Horizontal, i18n("Achternamen"));
    CustomSqlRelationalModel::setHeaderData(ORIGIN_ID, Qt::Horizontal, i18n("Oorsprong-id"));
    CustomSqlRelationalModel::setHeaderData(ORIGIN, Qt::Horizontal, i18n("Oorsprong"));

    CustomSqlRelationalModel::setSort(SORT, Qt::SortOrder::AscendingOrder);
}

NameOriginTableModel::NameOriginTableModel(QObject *parent) : QSqlTableModel(parent) {
    QSqlTableModel::setTable(Schema::NameOriginsTable);

    QSqlTableModel::setHeaderData(ID, Qt::Horizontal, i18n("Id"));
    QSqlTableModel::setHeaderData(ORIGIN, Qt::Horizontal, i18n("Oorsprong"));
}

Qt::ItemFlags NameOriginTableModel::flags(const QModelIndex &index) const {
    auto flags = QSqlTableModel::flags(index);
    if (index.column() == ID) {
        flags = flags & ~Qt::ItemIsEditable;
    }
    return flags;
}
