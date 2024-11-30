/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "person.h"

#include "names.h"

#include <KLocalizedString>
#include <QString>

QString Data::Person::Sex::toDisplay(const QString& sex) {
    if (sex == MALE) {
        return i18n("Man");
    }
    if (sex == FEMALE) {
        return i18n("Vrouw");
    }
    if (sex == UNKNOWN) {
        return i18n("Onbekend");
    }
    return sex;
}

QString Data::Person::Sex::toIcon(const QString& sex) {
    if (sex == MALE) {
        return QStringLiteral("♂");
    }
    if (sex == FEMALE) {
        return QStringLiteral("♀");
    }
    if (sex == UNKNOWN) {
        return QStringLiteral("?");
    }
    return QStringLiteral("⚥");
}

QVariant
OldDisplayNameProxyModel::extraColumnData(const QModelIndex& parent, int row, int extraColumn, int role) const {
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (extraColumn == 0) {
            const auto titles = this->index(row, 1, parent).data(role).toString();
            const auto givenNames = this->index(row, 2, parent).data(role).toString();
            const auto prefix = this->index(row, 3, parent).data(role).toString();
            const auto surname = this->index(row, 4, parent).data(role).toString();
            return construct_display_name(titles, givenNames, prefix, surname);
        }
    }
    return {};
}

OldDisplayNameProxyModel::OldDisplayNameProxyModel(QObject* parent) : KExtraColumnsProxyModel(parent) {
    this->appendColumn(i18n("Naam"));
}
