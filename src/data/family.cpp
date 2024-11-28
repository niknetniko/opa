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


FamilyProxyModel::FamilyProxyModel(IntegerPrimaryKey person, QObject* parent) : QAbstractProxyModel(parent) {
    query_ = QStringLiteral(R"-(
WITH parent_events(event_id) AS
       (SELECT events.id
        FROM events
               LEFT JOIN event_relations on events.id = event_relations.event_id
               LEFT JOIN event_roles on event_relations.role_id = event_roles.id
        WHERE person_id = %1
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
          AND event_relations.person_id != %1
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
WHERE event_relations.person_id != %1
  AND (events.id IN children OR events.id IN relationships)
  AND (names.sort = (SELECT MIN(n2.sort) FROM names AS n2 WHERE n2.person_id = event_relations.person_id));
)-")
                 .arg(person);

    QStringList roleValue;
    for (auto string: EventTypes::relationshipStartingEvents()) {
        roleValue.append(QString::fromUtf8(EventTypes::nameOriginToString[string].untranslatedText()));
    }
    roleValue.append(QString::fromUtf8(EventTypes::nameOriginToString[EventTypes::Birth].untranslatedText()));

    QSqlQuery query;
    query.prepare(query_);
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

    // TODO: check if this needs to happen when the underlying model changes.
    connect(this, &QAbstractProxyModel::sourceModelChanged, this, &FamilyProxyModel::updateMapping);

    QAbstractProxyModel::setSourceModel(baseModel);
}

QModelIndex FamilyProxyModel::parent(const QModelIndex& child) const {
    if (!child.isValid()) {
        return {};
    }

    auto sourceChild = mapToSource(child);
    for (auto [sourceParentRow, sourceChildRows]: mapping.asKeyValueRange()) {
        if (sourceChildRows.contains(sourceChild.row())) {
            return mapFromSource(sourceModel()->index(sourceParentRow, 0));
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

    int sourceRow = sourceIndex.row();
    auto keys = mapping.keys();
    // The source index refers to a parent.
    if (auto proxyRow = keys.indexOf(sourceRow); proxyRow != -1) {
        // qDebug() << "  refers to parent, returning parent index at" << proxyRow;
        return index(proxyRow, sourceIndex.column());
    }

    // We have a child, so the row is relative to the parent.
    for (auto it = mapping.cbegin(), end = mapping.cend(); it != end; ++it) {
        auto sourceParentRow = it.key();
        auto sourceChildRows = it.value();
        auto proxyParentIndex = std::distance(mapping.cbegin(), it);
        if (sourceChildRows.contains(sourceRow)) {
            int proxyChildRow = sourceChildRows.indexOf(sourceRow);
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
    int originalRow = proxyIndex.internalId();
    // qDebug() << "Mapping proxy index" << proxyIndex << "to source index";
    // qDebug() << "  internal source row is" << originalRow;
    return sourceModel()->index(originalRow, proxyIndex.column());
}

int FamilyProxyModel::rowCount(const QModelIndex& parent) const {
    // If we pass the top-level root, return the number of parents.
    if (!parent.isValid()) {
        // TODO: support another virtual row.
        // if (mapping.keys().contains(-1)) {
        //     // We do +1 since we manually add a top-level item.
        //     return static_cast<int>(mapping.keys().size() + 1);
        // } else {
        return static_cast<int>(mapping.keys().size());
        // }
    }

    // If the parent is one of the keys, it is a child.v
    auto sourceParent = mapToSource(parent);
    if (parent.column() == 0 && mapping.contains(sourceParent.row())) {
        return static_cast<int>(mapping[sourceParent.row()].size());
    }

    // In all other cases, either because it is not the right column, or we check more levels,
    // there are no children.
    return 0;
}

int FamilyProxyModel::columnCount(const QModelIndex& parent) const {
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
        auto originalRow = mapping[key][row];
        return createIndex(row, column, originalRow);
    } else {
        if (row >= keys.size()) {
            return {};
        }
        // We do not have a parent.
        auto originalRow = keys[row];
        return createIndex(row, column, originalRow);
    }
}

QVariant FamilyProxyModel::data(const QModelIndex& index, int role) const {
    // TODO: handle this
    // if (index.isValid() && !index.parent().isValid() && role == Qt::DisplayRole &&
    //     index.row() == rowCount(QModelIndex()) - 1) {
    //     // We are accessing the special parent row without actual parent.
    //     if (index.column() == GIVEN_NAMES) {
    //         return QStringLiteral("Out of wedlock");
    //     } else if (index.column() == TYPE) {
    //         return EventTypes::Values::Marriage;
    //     } else {
    //         return QStringLiteral("");
    //     }
    // }
    return QAbstractProxyModel::data(index, role);
}

QString FamilyProxyModel::query() const {
    return query_;
}

void FamilyProxyModel::updateMapping() {
    mapping.clear();
    QMap<QModelIndex, QList<QModelIndex>> internalMapping;

    if (sourceModel() == nullptr) {
        return; // There is no data.
    }

    qDebug() << " === SOURCE MODEL ===";
    // Print all source stuff for debug.
    for (int row = 0; row < sourceModel()->rowCount(); ++row) {
        // qDebug() << debugPrint(sourceModel(), row);
    }

    // First, map all parents to their index.
    QHash<IntegerPrimaryKey, QModelIndex> parentIdToRelationshipIndex;
    for (int row = 0; row < sourceModel()->rowCount(); ++row) {
        auto eventType = enumFromString<EventTypes::Values>(sourceModel()->index(row, TYPE).data().toString());
        auto personId = sourceModel()->index(row, PERSON_ID).data().toLongLong();
        auto eventId = sourceModel()->index(row, EVENT_ID).data().toLongLong();
        auto eventRole = enumFromString<EventRoles::Values>(sourceModel()->index(row, ROLE).data().toString());
        if (EventTypes::relationshipStartingEvents().contains(eventType)) {
            qDebug() << "Found partner relationship for event" << eventId << "with role" << eventRole;
            parentIdToRelationshipIndex[personId] = sourceModel()->index(row, 0);
        }
    }

    qDebug() << "Mapped all parents to their index:";
    qDebug() << "  " << parentIdToRelationshipIndex;

    // Now, we can map every birth event to a relationship index.
    QHash<IntegerPrimaryKey, QModelIndex> birthEventToParentRelationshipIndex;
    for (int row = 0; row < sourceModel()->rowCount(); ++row) {
        auto eventRole = enumFromString<EventRoles::Values>(sourceModel()->index(row, ROLE).data().toString());
        auto personId = sourceModel()->index(row, PERSON_ID).data().toLongLong();
        auto eventId = sourceModel()->index(row, EVENT_ID).data().toLongLong();
        if (EventRoles::parentRoles().contains(eventRole)) {
            qDebug() << "Found parent relationship for event" << eventId << "with role" << eventRole;
            auto parentRelationshipIndex = parentIdToRelationshipIndex[personId];
            birthEventToParentRelationshipIndex[eventId] = parentRelationshipIndex;
        }
    }

    qDebug() << "Mapped all births to their relationship index:";
    qDebug() << "   " << birthEventToParentRelationshipIndex;

    // Finally, we map every parent to their children.
    for (int row = 0; row < sourceModel()->rowCount(); ++row) {
        auto eventType = enumFromString<EventTypes::Values>(sourceModel()->index(row, TYPE).data().toString());
        auto eventId = sourceModel()->index(row, EVENT_ID).data().toLongLong();
        auto eventRole = enumFromString<EventRoles::Values>(sourceModel()->index(row, ROLE).data().toString());
        if (eventType == EventTypes::Birth && eventRole == EventRoles::Primary) {
            qDebug() << "Mapping birth event" << eventId << "with role" << eventRole;
            auto parentRelationshipIndex = birthEventToParentRelationshipIndex[eventId];
            qDebug() << "  the event is mapped to" << parentRelationshipIndex;
            internalMapping[parentRelationshipIndex].append(sourceModel()->index(row, 0));
        }
    }

    for (auto [key, value]: internalMapping.asKeyValueRange()) {
        int parentRow = key.row();
        // qDebug() << "PARENT" << debugPrint(sourceModel(), parentRow) << key;
        for (auto modelIndex: std::as_const(value)) {
            int childRow = modelIndex.row();
            // qDebug() << "  CHILD" << debugPrint(sourceModel(), childRow) << modelIndex;
        }
        qDebug() << key << value;
    }

    // TODO: do this directly.
    for (auto [key, value]: internalMapping.asKeyValueRange()) {
        int parentRow = key.row();
        for (auto modelIndex: std::as_const(value)) {
            int childRow = modelIndex.row();
            mapping[parentRow].append(childRow);
        }
    }

    qDebug() << "MAPPING IS ======================================";
    qDebug() << mapping;
}

// QString FamilyProxyModel::debugPrint(const QAbstractItemModel* model, int row) const {
//     if (row >= model->rowCount()) {
//         return QStringLiteral("[invalid]");
//     }
//     auto eventType = enumFromString<EventTypes::Values>(model->index(row, TYPE).data().toString());
//     auto personId = model->index(row, PERSON_ID).data().toLongLong();
//     auto eventId = sourceModel()->index(row, EVENT_ID).data().toLongLong();
//     auto eventRole = enumFromString<EventRoles::Values>(sourceModel()->index(row, ROLE).data().toString());
//
//     QString text;
//     QDebug{&text}.nospace() << "[id=" << eventId << ",type=" << eventType << ",person=" << personId
//                             << ",role=" << eventRole << "]";
//     return text;
// }
