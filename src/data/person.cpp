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
