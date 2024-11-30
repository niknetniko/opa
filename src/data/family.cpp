/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "family.h"

#include "event.h"
#include "utils/custom_sql_relational_model.h"

#include <QSqlError>
#include <QSqlQuery>

constexpr int BASTARD_CHILDREN_ROW_ID = std::numeric_limits<int>::max();


FamilyProxyModel::FamilyProxyModel(IntegerPrimaryKey person, QObject* parent) :
    QAbstractProxyModel(parent),
    person(person) {
    // TODO: is it needed to remove the hard-coded roles and relationships?
    query_ = QStringLiteral(R"-(
WITH parent_events(event_id) AS
       (SELECT events.id
        FROM events
               LEFT JOIN event_relations on events.id = event_relations.event_id
               LEFT JOIN event_roles on event_relations.role_id = event_roles.id
        WHERE person_id = :id
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
          AND event_relations.person_id != :id
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
WHERE event_relations.person_id != :id
  AND (events.id IN children OR events.id IN relationships)
  AND (names.sort = (SELECT MIN(n2.sort) FROM names AS n2 WHERE n2.person_id = event_relations.person_id));
)-");
    resetAndLoadData();
}

QModelIndex FamilyProxyModel::parent(const QModelIndex& child) const {
    if (!child.isValid()) {
        return {};
    }

    auto sourceChild = mapToSource(child);
    for (auto [sourceParentRow, sourceChildRows]: mapping.asKeyValueRange()) {
        if (sourceChildRows.contains(sourceChild.row())) {
            if (sourceParentRow == BASTARD_CHILDREN_ROW_ID && hasBastardChildren()) {
                return index(static_cast<int>(mapping.size() - 1), 0);
            } else {
                return mapFromSource(sourceModel()->index(sourceParentRow, 0));
            }
        }
    }

    return {};
}

bool FamilyProxyModel::hasChildren(const QModelIndex& parent) const {
    return rowCount(parent) > 0;
}

QModelIndex FamilyProxyModel::mapFromSource(const QModelIndex& sourceIndex) const {
    // qDebug() << "Mapping source index" << sourceIndex << "to proxy index.";
    if (!sourceIndex.isValid()) {
        return {};
    }

    const int sourceRow = sourceIndex.row();
    auto keys = mapping.keys();
    // The source index refers to a parent.
    if (auto proxyRow = keys.indexOf(sourceRow); proxyRow != -1) {
        // qDebug() << "  refers to parent, returning parent index at" << proxyRow;
        return index(static_cast<int>(proxyRow), sourceIndex.column());
    }

    // We have a child, so the row is relative to the parent.
    for (auto it = mapping.cbegin(), end = mapping.cend(); it != end; ++it) {
        const auto& sourceChildRows = it.value();
        auto proxyParentIndex = static_cast<int>(std::distance(mapping.cbegin(), it));
        if (sourceChildRows.contains(sourceRow)) {
            const int proxyChildRow = static_cast<int>(sourceChildRows.indexOf(sourceRow));
            // Get the parent for this child.
            auto parent = index(proxyParentIndex, 0);
            // qDebug() << "  refers to child, returning relative row" << proxyChildRow;
            return index(proxyChildRow, sourceIndex.column(), parent);
        }
    }

    return {};
}

QModelIndex FamilyProxyModel::mapToSource(const QModelIndex& proxyIndex) const {
    if (!proxyIndex.isValid()) {
        return {};
    }

    // Easy-peasy, get the row in the original model.
    const int originalRow = static_cast<int>(proxyIndex.internalId());
    return sourceModel()->index(originalRow, proxyIndex.column());
}

int FamilyProxyModel::rowCount(const QModelIndex& parent) const {
    // If we pass the top-level root, return the number of parents.
    if (!parent.isValid()) {
        return static_cast<int>(mapping.size());
    }

    const int sourceParentRow = hasBastardParent(parent) ? BASTARD_CHILDREN_ROW_ID : mapToSource(parent).row();

    // If the parent is one of the keys, it is a child.
    if (parent.column() == 0 && mapping.contains(sourceParentRow)) {
        return static_cast<int>(mapping[sourceParentRow].size());
    }

    // In all other cases, either because it is not the right column, or we check more levels,
    // there are no children.
    return 0;
}

int FamilyProxyModel::columnCount([[maybe_unused]] const QModelIndex& parent) const {
    return sourceModel()->columnCount();
}

QModelIndex FamilyProxyModel::index(int row, int column, const QModelIndex& parent) const {
    auto keys = mapping.keys();
    // Handle the case where we have a parent.
    if (parent.isValid() && parent.column() == 0) {
        if (parent.row() >= keys.size()) {
            return {};
        }
        // Find the parent in the mapping.
        auto key = keys[parent.row()];
        if (row >= mapping[key].size()) {
            return {};
        }
        auto originalRow = mapping[key].at(row);
        return createIndex(row, column, originalRow);
    } else {
        if (row >= keys.size()) {
            return {};
        }
        auto originalRow = keys[row];
        return createIndex(row, column, originalRow);
    }
}

QVariant FamilyProxyModel::data(const QModelIndex& index, int role) const {
    assert(checkIndex(index, CheckIndexOption::IndexIsValid));

    if (isBastardParentRow(index)) {
        if (role == Qt::DisplayRole) {
            switch (index.column()) {
                case TYPE:
                    return QStringLiteral("Children with no other parent");
                default:
                    return {};
            }
        } else {
            return {};
        }
    }

    return QAbstractProxyModel::data(index, role);
}

Qt::ItemFlags FamilyProxyModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void FamilyProxyModel::resetAndLoadData() {
    QSqlQuery query;
    query.prepare(query_);
    query.bindValue(QStringLiteral(":id"), person);

    if (!query.exec()) {
        qWarning() << "Something went wrong...";
        qDebug() << query.lastError();
    }

    auto* baseModel = new QSqlQueryModel(QObject::parent());
    baseModel->setQuery(std::move(query));
    baseModel->setHeaderData(EVENT_ID, Qt::Horizontal, i18n("Event ID"));
    baseModel->setHeaderData(ROLE_ID, Qt::Horizontal, i18n("Role ID"));
    baseModel->setHeaderData(ROLE, Qt::Horizontal, i18n("Role"));
    baseModel->setHeaderData(PERSON_ID, Qt::Horizontal, i18n("Person ID"));
    baseModel->setHeaderData(TYPE_ID, Qt::Horizontal, i18n("Type ID"));
    baseModel->setHeaderData(TYPE, Qt::Horizontal, i18n("Type"));
    baseModel->setHeaderData(DATE, Qt::Horizontal, i18n("Date"));
    baseModel->setHeaderData(GIVEN_NAMES, Qt::Horizontal, i18n("Given names"));
    baseModel->setHeaderData(PREFIX, Qt::Horizontal, i18n("Prefixes"));
    baseModel->setHeaderData(SURNAME, Qt::Horizontal, i18n("Surnames"));

    // TODO: check if this needs to happen when the underlying model changes.
    connect(this, &FamilyProxyModel::sourceModelChanged, this, &FamilyProxyModel::updateMapping);

    // We need at least one extra row.
    assert(baseModel->rowCount() < BASTARD_CHILDREN_ROW_ID - 1);
    QAbstractProxyModel::setSourceModel(baseModel);
}

void FamilyProxyModel::updateMapping() {
    mapping.clear();
    if (sourceModel() == nullptr) {
        return; // There is no data.
    }

    // First, map all parents to their index.
    QHash<IntegerPrimaryKey, QModelIndex> parentIdToRelationshipIndex;
    for (int row = 0; row < sourceModel()->rowCount(); ++row) {
        auto eventType = enumFromString<EventTypes::Values>(sourceModel()->index(row, TYPE).data().toString());
        if (EventTypes::relationshipStartingEvents().contains(eventType)) {
            auto personId = sourceModel()->index(row, PERSON_ID).data().toLongLong();
            parentIdToRelationshipIndex[personId] = sourceModel()->index(row, 0);
        }
    }

    // Now, we can map every birth event to a relationship index.
    QHash<IntegerPrimaryKey, int> birthEventToParentRelationshipIndex;
    for (int row = 0; row < sourceModel()->rowCount(); ++row) {
        auto eventRole = enumFromString<EventRoles::Values>(sourceModel()->index(row, ROLE).data().toString());
        if (EventRoles::parentRoles().contains(eventRole)) {
            auto eventId = sourceModel()->index(row, EVENT_ID).data().toLongLong();
            auto personId = sourceModel()->index(row, PERSON_ID).data().toLongLong();
            auto parentRelationshipIndex = parentIdToRelationshipIndex[personId];
            birthEventToParentRelationshipIndex[eventId] = parentRelationshipIndex.row();
        }
    }

    // Finally, we map every parent to their children.
    for (int sourceRow = 0; sourceRow < sourceModel()->rowCount(); ++sourceRow) {
        auto eventType = enumFromString<EventTypes::Values>(sourceModel()->index(sourceRow, TYPE).data().toString());
        auto eventRole = enumFromString<EventRoles::Values>(sourceModel()->index(sourceRow, ROLE).data().toString());
        if (eventType == EventTypes::Birth && eventRole == EventRoles::Primary) {
            auto eventId = sourceModel()->index(sourceRow, EVENT_ID).data().toLongLong();
            auto parentRelationshipRow = birthEventToParentRelationshipIndex.value(eventId, BASTARD_CHILDREN_ROW_ID);
            mapping[parentRelationshipRow].append(sourceRow);
        }
    }
}

bool FamilyProxyModel::hasBastardChildren() const {
    return mapping.contains(BASTARD_CHILDREN_ROW_ID);
}

bool FamilyProxyModel::isBastardParentRow(const QModelIndex& index) const {
    return !index.parent().isValid() && hasBastardParent(index);
}

bool FamilyProxyModel::hasBastardParent(const QModelIndex& parent) const {
    return hasBastardChildren() && parent.row() + 1 == mapping.size();
}
