/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "utils/custom_sql_relational_model.h"
#include "utils/model_utils_find_source_model_of_type.h"

#include <QString>

namespace EventRoles {
    Q_NAMESPACE;

    enum Values {
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
        {Primary, kli18n("Primary")},
        {Partner, kli18n("Partner")},
        {Witness, kli18n("Witness")},
        {Mother, kli18n("Mother")},
        {Father, kli18n("Father")},
        {AdoptiveParent, kli18n("Adoptive parent")},
        {Stepparent, kli18n("Stepparent")},
        {FosterParent, kli18n("Foster parent")},
        {SurrogateMother, kli18n("Surrogate mother")},
        {GeneticDonor, kli18n("Genetic donor")},
        {RecognizedParent, kli18n("Recognized parent")},
    };

    const std::function toDisplayString = [](const QString& databaseValue) {
        return genericToDisplayString<Values>(databaseValue, nameOriginToString);
    };

    QList<Values> parentRoles();

} // namespace EventRoles

class EventRolesModel : public QSqlTableModel {
    Q_OBJECT

public:
    static constexpr int ID = 0;
    static constexpr int ROLE = 1;
    static constexpr int BUILTIN = 2;

    static QVariant getDefaultRole();

    explicit EventRolesModel(QObject* parent);
};

namespace EventTypes {
    Q_NAMESPACE

    enum Values { Birth, Death, Marriage, Divorce, Baptism, Funeral };

    Q_ENUM_NS(Values);

    const QHash<Values, KLazyLocalizedString> typeToString{
        {Birth, kli18n("Birth")},
        {Death, kli18n("Death")},
        {Marriage, kli18n("Marriage")},
        {Divorce, kli18n("Divorce")},
        {Baptism, kli18n("Baptism")},
        {Funeral, kli18n("Funeral")}
    };

    const std::function toDisplayString = [](const QString& databaseValue) {
        return genericToDisplayString<Values>(databaseValue, typeToString);
    };

    /**
     * A list of event types that start a relationship which can lead to a family.
     */
    QList<Values> relationshipStartingEvents();

} // namespace EventTypes

class EventTypesModel : public QSqlTableModel {
    Q_OBJECT

public:
    static constexpr int ID = 0;
    static constexpr int TYPE = 1;
    static constexpr int BUILTIN = 2;

    explicit EventTypesModel(QObject* parent);
};

class EventRelationsModel : public CustomSqlRelationalModel {
    Q_OBJECT

public:
    static constexpr int EVENT_ID = 0;
    static constexpr int PERSON_ID = 1;
    static constexpr int ROLE_ID = 2;
    static constexpr int ROLE = 3;

    explicit EventRelationsModel(QObject* parent, QSqlTableModel* rolesModel);
};

class EventsModel : public CustomSqlRelationalModel {
    Q_OBJECT

public:
    static constexpr int ID = 0;
    static constexpr int TYPE_ID = 1;
    static constexpr int DATE = 2;
    static constexpr int NAME = 3;
    static constexpr int NOTE = 4;
    // Extra columns added by the relation.
    static constexpr int TYPE = 5;

    explicit EventsModel(QObject* parent, QSqlTableModel* typesModel);
};

namespace PersonEventsModel {
    constexpr int ROLE = 0;
    constexpr int TYPE = 1;
    constexpr int DATE = 2;
    constexpr int NAME = 3;
    constexpr int ID = 4;
    constexpr int ROLE_ID = 5;
} // namespace PersonEventsModel
