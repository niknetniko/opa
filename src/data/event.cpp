/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "event.h"

QList<EventTypes::Values> EventTypes::relationshipStartingEvents() {
    return {Marriage};
}

QList<EventTypes::Values> EventTypes::birthEventsInOrder() {
    return {Birth, Baptism};
}

QList<EventTypes::Values> EventTypes::deathEventsInOrder() {
    return {Death, Funeral};
}

QList<EventRoles::Values> EventRoles::parentRoles() {
    return {Mother, Father, AdoptiveParent, Stepparent, FosterParent, SurrogateMother, GeneticDonor, RecognizedParent};
}
