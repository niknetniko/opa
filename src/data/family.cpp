// SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "family.h"

#include "event.h"
#include "utils/custom_sql_relational_model.h"

#include <QSqlError>
#include <QSqlQuery>

FamilyProxyModel::FamilyProxyModel(IntegerPrimaryKey person, QObject* parent) : QAbstractProxyModel(parent) {

    auto queryString = QStringLiteral(R"-(
WITH parent_events(event_id) AS
       (SELECT events.id
        FROM events
               LEFT JOIN event_relations on events.id = event_relations.event_id
               LEFT JOIN event_roles on event_relations.role_id = event_roles.id
        WHERE person_id = 1
          AND event_roles.role IN ('Father', 'Mother')),
     children(event_id) AS
       (SELECT events.id
        FROM events
               LEFT JOIN event_relations on events.id = event_relations.event_id
               LEFT JOIN event_roles on event_relations.role_id = event_roles.id
               LEFT JOIN event_types on event_types.id = events.type_id
        WHERE event_types.type = 'Birth'
          AND event_roles.role = 'Primary'
          AND events.id IN parent_events),
     parents_of_children(person_id) AS
       (SELECT event_relations.person_id
        FROM events
               LEFT JOIN event_relations on events.id = event_relations.event_id
               LEFT JOIN event_roles on event_relations.role_id = event_roles.id
               LEFT JOIN event_types on event_types.id = events.type_id
        WHERE event_roles.role IN ('Father', 'Mother')
          AND event_relations.person_id != 1
          AND events.id IN parent_events),
     relationships(event_id) AS
       (SELECT events.id
        FROM events
               LEFT JOIN event_relations on events.id = event_relations.event_id
               LEFT JOIN event_roles on event_relations.role_id = event_roles.id
               LEFT JOIN event_types on event_types.id = events.type_id
        WHERE event_types.type IN ('Marriage')
          AND event_roles.role IN ('Primary', 'Partner')
          AND event_relations.person_id IN parents_of_children)
SELECT events.id,
       event_roles.id,
       event_roles.role,
       event_relations.person_id,
       event_types.id,
       event_types.type,
       events.date,
       names.given_names,
       names.prefix,
       names.surname
FROM events
       LEFT JOIN event_relations ON events.id = event_relations.event_id
       LEFT JOIN event_roles ON event_roles.id = event_relations.role_id
       LEFT JOIN event_types ON events.type_id = event_types.id
       LEFT JOIN names on event_relations.person_id = names.person_id
WHERE events.id IN children
   OR events.id IN relationships
  AND (names.sort = (SELECT MIN(n2.sort) FROM names AS n2 WHERE n2.person_id = event_relations.person_id));
)-");

    QStringList roleValue;
    for (auto string: EventTypes::relationshipStartingEvents()) {
        roleValue.append(QString::fromUtf8(EventTypes::nameOriginToString[string].untranslatedText()));
    }
    roleValue.append(QString::fromUtf8(EventTypes::nameOriginToString[EventTypes::Birth].untranslatedText()));

    QSqlQuery query;
    query.prepare(queryString);
    query.bindValue(QStringLiteral(":types"), roleValue.join(QStringLiteral(",")));
    query.bindValue(QStringLiteral(":id"), person);

    if (!query.exec()) {
        qWarning() << "Something went wrong...";
        qDebug() << query.lastError();
    }

    auto* baseModel = new QSqlQueryModel(parent);
    baseModel->setQuery(std::move(query));
    baseModel->setHeaderData(EVENT_ID, Qt::Horizontal, i18n("Id"));
    baseModel->setHeaderData(ROLE_ID, Qt::Horizontal, i18n("Rol-id"));
    baseModel->setHeaderData(ROLE, Qt::Horizontal, i18n("Rol"));
    baseModel->setHeaderData(PERSON_ID, Qt::Horizontal, i18n("Persoon-id"));
    baseModel->setHeaderData(TYPE_ID, Qt::Horizontal, i18n("Soort-id"));
    baseModel->setHeaderData(TYPE, Qt::Horizontal, i18n("Soort"));
    baseModel->setHeaderData(DATE, Qt::Horizontal, i18n("Datum"));
    baseModel->setHeaderData(GIVEN_NAMES, Qt::Horizontal, i18n("Voornamen"));
    baseModel->setHeaderData(PREFIX, Qt::Horizontal, i18n("Voorvoegsels"));
    baseModel->setHeaderData(SURNAME, Qt::Horizontal, i18n("Achternamen"));

    QAbstractProxyModel::setSourceModel(baseModel);
}

QModelIndex FamilyProxyModel::parent(const QModelIndex& child) const {
    if (child.column() != 0) {
        // By convention, only the first column has parents.
        return {};
    }

    auto sourceChild = mapToSource(child);
    auto mapping = createMapping();
    assert(sourceChild.column() == 0);
    for (auto i = mapping.cbegin(), end = mapping.cend(); i != end; ++i) {
        if (i.value().contains(sourceChild)) {
            return mapFromSource(i.key());
        }
    }

    return {};
}

QModelIndex FamilyProxyModel::mapFromSource(const QModelIndex& sourceIndex) const {
    // Check if the source index is a parent or not.
    auto mapping = createMapping();
    auto firstSourceMapping = sourceModel()->index(sourceIndex.row(), 0);
    auto keys = mapping.keys();
    auto positionInKeys = keys.indexOf(firstSourceMapping);
    if (positionInKeys != -1) {
        return createIndex(positionInKeys, sourceIndex.column());
    } else {
        // We have a child, so look up the child...
        for (auto i = mapping.cbegin(), end = mapping.cend(); i != end; ++i) {
            if (i.value().contains(sourceIndex)) {
                int row = i.value().indexOf(sourceIndex);
                return createIndex(row, sourceIndex.column());
            }
        }
    }

    // Euh?
    return {};
}

QModelIndex FamilyProxyModel::mapToSource(const QModelIndex& proxyIndex) const {
    auto proxyParent = parent(proxyIndex);
    auto mapping = createMapping();
    if (proxyParent.isValid()) {
        auto sourceParent = mapToSource(proxyParent);
        auto children = mapping[sourceParent];
        auto sourceFirstIndex = children[proxyIndex.row()];
        return sourceModel()->index(sourceFirstIndex.row(), proxyIndex.column());
    } else {
        // It has no parent, so the proxy index's row is the index of the map key.
        auto sourceFirstIndex = mapping.keys()[proxyIndex.row()];
        return sourceModel()->index(sourceFirstIndex.row(), proxyIndex.column());
    }
}

int FamilyProxyModel::rowCount(const QModelIndex& parent) const {
    auto mapping = createMapping();
    if (parent.isValid()) {
        auto sourceParent = mapToSource(parent);
        return mapping[sourceParent].size();
    } else {
        return mapping.keys().size();
    }
}

QMap<QModelIndex, QList<QModelIndex>> FamilyProxyModel::createMapping() const {
    QMap<QModelIndex, QList<QModelIndex>> mapping;

    if (sourceModel() == nullptr) {
        return mapping; // There is no data.
    }

    // First, map all parents to their index.
    QHash<IntegerPrimaryKey, QModelIndex> parentIdToRelationshipIndex;
    for (int row = 0; row < sourceModel()->rowCount(); ++row) {
        auto eventType = enumFromString<EventTypes::Values>(sourceModel()->index(row, TYPE).data().toString());
        auto personId = sourceModel()->index(row, PERSON_ID).data().toLongLong();
        if (EventTypes::relationshipStartingEvents().contains(eventType)) {
            parentIdToRelationshipIndex[personId] = sourceModel()->index(row, 0);
        }
    }

    // Now, we can map every birth event to a relationship index.
    QHash<IntegerPrimaryKey, QModelIndex> birthEventToParentRelationshipIndex;
    for (int row = 0; row < sourceModel()->rowCount(); ++row) {
        auto eventRole = enumFromString<EventRoles::Values>(sourceModel()->index(row, ROLE).data().toString());
        auto personId = sourceModel()->index(row, PERSON_ID).data().toLongLong();
        auto eventId = sourceModel()->index(row, EVENT_ID).data().toLongLong();
        if (EventRoles::parentRoles().contains(eventRole)) {
            auto parentRelationshipIndex = parentIdToRelationshipIndex[personId];
            birthEventToParentRelationshipIndex[eventId] = parentRelationshipIndex;
        }
    }

    // Finally, we can now map all births with children to their parent relationship.
    for (int row = 0; row < sourceModel()->rowCount(); ++row) {
        auto eventType = enumFromString<EventTypes::Values>(sourceModel()->index(row, TYPE).data().toString());
        auto eventId = sourceModel()->index(row, EVENT_ID).data().toLongLong();
        if (eventType == EventTypes::Birth) {
            auto parentRelationshipIndex = birthEventToParentRelationshipIndex[eventId];
            mapping[parentRelationshipIndex].append(sourceModel()->index(row, 0));
        }
    }

    return mapping;
}
