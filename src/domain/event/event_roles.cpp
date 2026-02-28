/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "event_roles.h"


QList<EventRoles::Values> EventRoles::parentRoles() {
    return {
        Values::Mother,
        Values::Father,
        Values::AdoptiveParent,
        Values::Stepparent,
        Values::FosterParent,
        Values::SurrogateMother,
        Values::GeneticDonor,
        Values::RecognizedParent
    };
}
