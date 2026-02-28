/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "event_types.h"

QList<EventTypes::Values> EventTypes::relationshipStartingEvents() {
    return {Values::Marriage};
}

QList<EventTypes::Values> EventTypes::birthEventsInOrder() {
    return {Values::Birth, Values::Baptism};
}

QList<EventTypes::Values> EventTypes::deathEventsInOrder() {
    return {Values::Death, Values::Funeral};
}
