/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "names.h"

#include "database/schema.h"

QString construct_display_name(
    const QString& titles, const QString& givenNames, const QString& prefix, const QString& surname
) {
    QStringList nameParts;
    if (!titles.isEmpty()) {
        nameParts.append(titles);
    }
    if (!givenNames.isEmpty()) {
        nameParts.append(givenNames);
    }
    if (!prefix.isEmpty()) {
        nameParts.append(prefix);
    }
    if (!surname.isEmpty()) {
        nameParts.append(surname);
    }
    return nameParts.join(QStringLiteral(" "));
}

NamesTableModel::NamesTableModel(QObject* parent, QSqlTableModel* originsModel) : CustomSqlRelationalModel(parent) {
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
    CustomSqlRelationalModel::setHeaderData(NOTE, Qt::Horizontal, i18n("Notitie"));

    CustomSqlRelationalModel::setSort(SORT, Qt::SortOrder::AscendingOrder);
}

NameOriginTableModel::NameOriginTableModel(QObject* parent) : QSqlTableModel(parent) {
    QSqlTableModel::setTable(Schema::NameOriginsTable);

    QSqlTableModel::setHeaderData(ID, Qt::Horizontal, i18n("Id"));
    QSqlTableModel::setHeaderData(ORIGIN, Qt::Horizontal, i18n("Oorsprong"));
    QSqlTableModel::setHeaderData(BUILTIN, Qt::Horizontal, i18n("Ingebouwd"));
}
