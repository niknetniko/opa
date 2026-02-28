/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "person_sex.h"

#include <QString>

QString Sex::toIcon(const QString& sex) {
    if (!isValidEnum<Values>(sex)) {
        return QStringLiteral("⚥");
    }

    switch (enumFromString<Values>(sex)) {
        case Values::Male:
            return QStringLiteral("♂");
        case Values::Female:
            return QStringLiteral("♀");
        case Values::Unknown:
        default:
            return QStringLiteral("?");
    }
}
