/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "utils/model_utils.h"

#include <QString>

namespace EventRoles {
Q_NAMESPACE;

enum class Values {
    /**
     * The primary participant in the event.
     * Its role depends on the event in question.
     * For example, the newborn is the primary in birth, while the deceased is the primary in a death event.
     * One exception are marriages and other partnership events: by convention, the person with the lowest ID
     * is the primary, the other one is called "Partner".
     */
    Primary,
    /**
     * Role for the second person in relationship events.
     */
    Partner,
    /**
     * A generic role for a secondary participant who plays no other role than being present.
     */
    Witness,
    // The following roles are for birth events only.
    /**
     * The biological mother of the child.
     * Role should only be used with birth events.
     */
    Mother,
    /**
     * The biological father of the child.
     * Role should only be used with birth events.
     */
    Father,
    /**
     * The adoptive parent of the child.
     * Role should only be used with birth events.
     * Based on the sex of the linked person, it becomes the adoptive mother or father.
     */
    AdoptiveParent,
    /**
     * A stepparent of the child.
     * Role should only be used with birth events.
     * Based on the sex of the linked person, it becomes the stepmother or stepfather.
     */
    Stepparent,
    /**
     * A foster parent of the child.
     * Role should only be used with birth events.
     * Based on the sex of the linked person, it becomes the foster mother or father.
     */
    FosterParent,
    /**
     * Surrogate mother.
     * Role should only be used with birth events.
     * Based on other roles, the surrogate is assumed to not have donated genetic material.
     * For example, if another woman is linked as GeneticDonor or Mother, it is assumed the surrogate did not
     * donate any genetic material to the child.
     */
    SurrogateMother,
    /**
     * Sperm or egg donor of the child.
     * Role should only be used with birth events.
     * Based on the sex of the linked person, it is either a sperm donor or egg donor.
     */
    GeneticDonor,
    /**
     * Indicates that the person is recognized as the parent of the child (the primary participant of the event),
     * without having been through e.g. adoption procedures.
     * This encompasses alternative parenting cultures or legal parents of which it is known that they are not the
     * biological father.
     * For example, if a man is revealed not to be the biological father, but he still is the legal parent of the
     * child.
     *
     */
    RecognizedParent,
};

Q_ENUM_NS(Values);

const QHash<Values, KLazyLocalizedString> nameOriginToString{
    {Values::Primary, kli18n("Primary")},
    {Values::Partner, kli18n("Partner")},
    {Values::Witness, kli18n("Witness")},
    {Values::Mother, kli18n("Mother")},
    {Values::Father, kli18n("Father")},
    {Values::AdoptiveParent, kli18n("Adoptive parent")},
    {Values::Stepparent, kli18n("Stepparent")},
    {Values::FosterParent, kli18n("Foster parent")},
    {Values::SurrogateMother, kli18n("Surrogate mother")},
    {Values::GeneticDonor, kli18n("Genetic donor")},
    {Values::RecognizedParent, kli18n("Recognized parent")},
};

const auto toDisplayString = [](const QString& databaseValue) {
    return genericToDisplayString<Values>(databaseValue, nameOriginToString);
};

QList<Values> parentRoles();
}
