/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "source.h"

#include "database/schema.h"

#include <KLocalizedString>

SourcesTableModel::SourcesTableModel(QObject* parent) : QSqlTableModel(parent) {
    QSqlTableModel::setTable(Schema::SourcesTable);

    QSqlTableModel::setHeaderData(ID, Qt::Horizontal, i18n("Id"));
    QSqlTableModel::setHeaderData(TITLE, Qt::Horizontal, i18n("Title"));
    QSqlTableModel::setHeaderData(TYPE, Qt::Horizontal, i18n("Type"));
    QSqlTableModel::setHeaderData(AUTHOR, Qt::Horizontal, i18n("Author"));
    QSqlTableModel::setHeaderData(PUBLICATION, Qt::Horizontal, i18n("Publication"));
    QSqlTableModel::setHeaderData(CONFIDENCE, Qt::Horizontal, i18n("Confidence"));
    QSqlTableModel::setHeaderData(NOTE, Qt::Horizontal, i18n("Note"));
    QSqlTableModel::setHeaderData(PARENT_ID, Qt::Horizontal, i18n("Parent ID"));
}
