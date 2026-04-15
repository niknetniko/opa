/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "utils/model_utils.h"

#include <QString>

namespace LocationTypes {
Q_NAMESPACE

enum class Values { Country, Province, County, City, Village, Parish, Address };

Q_ENUM_NS(Values);

const QHash<Values, KLazyLocalizedString> typeToString{
    {Values::Country, kli18n("Country")},
    {Values::Province, kli18n("Province")},
    {Values::County, kli18n("County")},
    {Values::City, kli18n("City")},
    {Values::Village, kli18n("Village")},
    {Values::Parish, kli18n("Parish")},
    {Values::Address, kli18n("Address")},
};

const auto toDisplayString = [](const QString& databaseValue) -> QString {
    if (!isValidEnum<Values>(databaseValue)) {
        return databaseValue;
    }
    return genericToDisplayString<Values>(databaseValue, typeToString);
};

} // namespace LocationTypes
