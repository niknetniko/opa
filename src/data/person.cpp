/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "person.h"

#include "database/schema.h"

#include <QString>

QString Sex::toIcon(const QString& sex) {
    if (!isValidEnum<Values>(sex)) {
        return QStringLiteral("⚥");
    }

    switch (enumFromString<Values>(sex)) {
        case Male:
            return QStringLiteral("♂");
        case Female:
            return QStringLiteral("♀");
        case Unknown:
        default:
            return QStringLiteral("?");
    }
}

PeopleTableModel::PeopleTableModel(QObject* parent) : QSqlTableModel(parent) {
    QSqlTableModel::setTable(Schema::PeopleTable);

    QSqlTableModel::setHeaderData(ID, Qt::Horizontal, i18n("Id"));
    QSqlTableModel::setHeaderData(ROOT, Qt::Horizontal, i18n("Root"));
    QSqlTableModel::setHeaderData(SEX, Qt::Horizontal, i18n("Sex"));
}
