/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "person.h"

#include "names.h"

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
