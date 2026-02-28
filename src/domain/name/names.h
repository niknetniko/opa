/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "utils/model_utils.h"

#include <KLazyLocalizedString>
#include <QString>

/**
 * Create a single display name from the different name parts.
 *
 * Any of the parts can be empty.
 */
QString
construct_display_name(const QString& titles, const QString& givenNames, const QString& prefix, const QString& surname);

namespace NameOrigins {
Q_NAMESPACE

enum class Values { Unknown, Inherited, Patrilineal, Matrilineal, Taken, Patronymic, Matronymic, Location, Occupation };

Q_ENUM_NS(Values)

const QHash<Values, KLazyLocalizedString> nameOriginToString = {
    {Values::Unknown, kli18n("Unknown")},
    {Values::Inherited, kli18n("Inherited")},
    {Values::Patrilineal, kli18n("Patrilineal")},
    {Values::Matrilineal, kli18n("Matrilineal")},
    {Values::Taken, kli18n("Taken")},
    {Values::Patronymic, kli18n("Patronymic")},
    {Values::Matronymic, kli18n("Matronymic")},
    {Values::Location, kli18n("Location")},
    {Values::Occupation, kli18n("Occupation")}
};

const auto toDisplayString = [](const QString& databaseValue) {
    return genericToDisplayString<Values>(databaseValue, nameOriginToString);
};
}
