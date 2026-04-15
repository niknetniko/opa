/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "utils/model_utils.h"

#include <QString>

namespace EventTypes {
Q_NAMESPACE

enum class Values { Birth, Death, Marriage, Divorce, Baptism, Funeral };

Q_ENUM_NS(Values);

const QHash<Values, KLazyLocalizedString> typeToString{
    {Values::Birth, kli18n("Birth")},
    {Values::Death, kli18n("Death")},
    {Values::Marriage, kli18n("Marriage")},
    {Values::Divorce, kli18n("Divorce")},
    {Values::Baptism, kli18n("Baptism")},
    {Values::Funeral, kli18n("Funeral")}
};

const auto toDisplayString = [](const QString& databaseValue) -> QString {
    if (!isValidEnum<Values>(databaseValue)) {
        return databaseValue;
    }
    return genericToDisplayString<Values>(databaseValue, typeToString);
};

/**
 * A list of event types that start a relationship which can lead to a family.
 */
QList<Values> relationshipStartingEvents();

/**
 * Get the "birth" events type in the order they should be considered.
 * For example, if there is no birth event, a baptism event may be considered.
 */
QList<Values> birthEventsInOrder();

/**
 * Get the "death" events type in the order they should be considered.
 * For example, if there is no death event, a burial event may be considered.
 */
QList<Values> deathEventsInOrder();
}
