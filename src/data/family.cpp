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
WITH parent_events AS
       (SELECT events.id AS event_id
        FROM events
               JOIN event_relations AS parent_relation ON events.id = parent_relation.event_id
               JOIN event_roles ON parent_relation.role_id = event_roles.id
        WHERE parent_relation.person_id = :id
          AND event_roles.role IN ('Father', 'Mother')),
     children AS
       (SELECT child_relation.person_id AS child_id,
               events.id                AS birth_event_id,
               events.date              AS birth_date,
               events.type_id           AS birth_type_id,
               event_types.type         AS birth_type
        FROM events
               JOIN event_relations AS child_relation ON events.id = child_relation.event_id
               JOIN event_roles ON child_relation.role_id = event_roles.id
               JOIN event_types ON events.type_id = event_types.id
        WHERE event_types.type = 'Birth'
          AND event_roles.role = 'Primary'
          AND events.id IN (SELECT event_id FROM parent_events)),
     parents_of_children AS
       (SELECT DISTINCT parent_relation.person_id AS partner_id,
                        child_relation.person_id  AS child_id
        FROM event_relations AS parent_relation
               JOIN event_relations AS child_relation ON parent_relation.event_id = child_relation.event_id
               JOIN event_roles ON parent_relation.role_id = event_roles.id
        WHERE parent_relation.event_id IN (SELECT event_id FROM parent_events)
          AND event_roles.role IN ('Father', 'Mother')
          AND parent_relation.person_id != :id),
     relationships AS
       (SELECT events.id                 AS marriage_event_id,
               events.date               AS marriage_date,
               parent_relation.person_id AS partner_id,
               events.type_id            AS marriage_type_id,
               event_types.type          AS marriage_type
        FROM events
               JOIN event_relations AS parent_relation ON events.id = parent_relation.event_id
               JOIN event_roles ON parent_relation.role_id = event_roles.id
               JOIN event_types ON events.type_id = event_types.id
        WHERE event_types.type = 'Marriage'
          AND parent_relation.person_id IN (SELECT partner_id FROM parents_of_children))
SELECT child.birth_type     AS event_type,
       child.birth_event_id AS event_type_id,
       child.child_id       AS person_id,
       parent.partner_id    AS partner_id,
       child.birth_event_id AS event_id,
       child.birth_date     AS event_date,
       names.titles         AS titles,
       names.given_names    AS given_names,
       names.prefix         AS prefix,
       names.surname        AS surname
FROM children AS child
       LEFT JOIN parents_of_children AS parent ON child.child_id = parent.child_id
       LEFT JOIN names ON child.child_id = names.person_id
WHERE names.sort = (SELECT MIN(n2.sort) FROM names AS n2 WHERE n2.person_id = child.child_id) OR names.sort = NULL

UNION ALL

SELECT marriage.marriage_type     AS event_type,
       marriage.marriage_type_id  AS event_type_id,
       marriage.partner_id        AS person_id,
       NULL                       AS partner_id,
       marriage.marriage_event_id AS event_id,
       marriage.marriage_date     AS event_date,
       names.titles               AS titles,
       names.given_names          AS given_names,
       names.prefix               AS prefix,
       names.surname              AS surname
FROM relationships AS marriage
       LEFT JOIN names ON marriage.partner_id = names.person_id
WHERE names.sort = (SELECT MIN(n2.sort) FROM names AS n2 WHERE n2.person_id = marriage.partner_id) OR names.sort = NULL

ORDER BY event_type, event_date;
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

    const int sourceParentRow = isBastardParentRow(parent) ? BASTARD_CHILDREN_ROW_ID : mapToSource(parent).row();

    // If the parent is one of the keys, it is a child.
    if (parent.column() == 0 && mapping.contains(sourceParentRow)) {
        return static_cast<int>(mapping[sourceParentRow].size());
    }

    // In all other cases, either because it is not the right column, or we check more levels,
    // there are no children.
    return 0;
}

int FamilyProxyModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
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
    Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid));

    if (isBastardParentRow(index)) {
        if (role == Qt::DisplayRole) {
            switch (index.column()) {
                case EVENT_TYPE:
                    return QStringLiteral("Children with no other parent");
                default:
                    return QStringLiteral("");
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

QVariant FamilyProxyModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && mapping.size() == 1) {
        auto nonBastardRowIndex = index(0, section, index(0, 0));
        int sourceSection = mapToSource(nonBastardRowIndex).column();
        return sourceModel()->headerData(sourceSection, orientation, role);
    }

    return QAbstractProxyModel::headerData(section, orientation, role);
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
    baseModel->setHeaderData(EVENT_TYPE, Qt::Horizontal, i18n("Event type"));
    baseModel->setHeaderData(EVENT_TYPE_ID, Qt::Horizontal, i18n("Event type ID"));
    baseModel->setHeaderData(PERSON_ID, Qt::Horizontal, i18n("Person ID"));
    baseModel->setHeaderData(PARTNER_ID, Qt::Horizontal, i18n("Partner ID"));
    baseModel->setHeaderData(EVENT_ID, Qt::Horizontal, i18n("Event ID"));
    baseModel->setHeaderData(DATE, Qt::Horizontal, i18n("Date"));
    baseModel->setHeaderData(TITLES, Qt::Horizontal, i18n("Titles"));
    baseModel->setHeaderData(GIVEN_NAMES, Qt::Horizontal, i18n("Given names"));
    baseModel->setHeaderData(PREFIX, Qt::Horizontal, i18n("Prefixes"));
    baseModel->setHeaderData(SURNAME, Qt::Horizontal, i18n("Surnames"));

    // TODO: check if this needs to happen when the underlying model changes.
    connect(this, &FamilyProxyModel::sourceModelChanged, this, &FamilyProxyModel::updateMapping);

    // We need at least one extra row.
    Q_ASSERT(baseModel->rowCount() < BASTARD_CHILDREN_ROW_ID - 1);
    QAbstractProxyModel::setSourceModel(baseModel);
}

void FamilyProxyModel::updateMapping() {
    mapping.clear();
    if (sourceModel() == nullptr) {
        return; // There is no data.
    }

    // Mapping of person ID -> row of the marriage event.
    QHash<IntegerPrimaryKey, int> personIdToRelationshipRow;
    for (int row = 0; row < sourceModel()->rowCount(); ++row) {
        auto eventType = enumFromString<EventTypes::Values>(sourceModel()->index(row, EVENT_TYPE).data().toString());
        if (EventTypes::relationshipStartingEvents().contains(eventType)) {
            auto personId = sourceModel()->index(row, PERSON_ID).data().toLongLong();
            personIdToRelationshipRow[personId] = row;
        }
    }

    // Now map each birth event to their parent.
    for (int sourceRow = 0; sourceRow < sourceModel()->rowCount(); ++sourceRow) {
        auto eventType =
            enumFromString<EventTypes::Values>(sourceModel()->index(sourceRow, EVENT_TYPE).data().toString());
        if (eventType == EventTypes::Birth) {
            auto partnerIdData = sourceModel()->index(sourceRow, PARTNER_ID).data();
            int partnerId = -1;
            if (!partnerIdData.isNull()) {
                partnerId = partnerIdData.toInt();
                Q_ASSERT(personIdToRelationshipRow.contains(partnerId));
            }
            auto parentRelationshipRow = personIdToRelationshipRow.value(partnerId, BASTARD_CHILDREN_ROW_ID);
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

AncestorQueryModel::AncestorQueryModel(IntegerPrimaryKey person, QObject* parent) :
    QSqlQueryModel(parent),
    person(person) {
    this->query_ = QStringLiteral(R"-(
WITH RECURSIVE
  parent_of(child_id, father_id, mother_id) AS
    (SELECT child_relation.person_id  AS child_id,
            father_relation.person_id AS father_id,
            mother_relation.person_id AS mother_id
     FROM event_relations AS child_relation
            LEFT JOIN event_relations AS father_relation
                      ON child_relation.event_id = father_relation.event_id
                        AND father_relation.role_id = (SELECT id FROM event_roles WHERE role = 'Father')
            LEFT JOIN event_relations AS mother_relation
                      ON child_relation.event_id = mother_relation.event_id
                        AND mother_relation.role_id = (SELECT id FROM event_roles WHERE role = 'Mother')
     WHERE child_relation.role_id = (SELECT id FROM event_roles WHERE role = 'Primary')
     GROUP BY child_relation.person_id

     UNION ALL

     SELECT :person, NULL, NULL),
  ancestor_tree(child_id, father_id, mother_id, visited) AS
    (SELECT child_id,
            father_id,
            mother_id,
            CAST(child_id AS TEXT) AS visited
     FROM parent_of
     WHERE child_id = :person

     UNION ALL

     SELECT parent_of.child_id                                 AS child_id,
            parent_of.father_id                                AS father_id,
            parent_of.mother_id                                AS mother_id,
            ancestor_tree.visited || ',' || parent_of.child_id AS visited
     FROM ancestor_tree
            LEFT JOIN parent_of
                      ON parent_of.child_id = ancestor_tree.father_id
                        OR parent_of.child_id = ancestor_tree.mother_id
     WHERE parent_of.child_id IS NOT NULL
       AND ',' || ancestor_tree.visited || ',' NOT LIKE '%,' || parent_of.child_id || ',%')
SELECT child_id,
       father_id,
       mother_id,
       visited,
       json_array_length('[' || visited || ']') AS level,
       names.titles,
       names.given_names,
       names.prefix,
       names.surname
FROM ancestor_tree
       LEFT JOIN names ON ancestor_tree.child_id = names.person_id
WHERE names.sort = (SELECT MIN(n2.sort) FROM names AS n2 WHERE n2.person_id = ancestor_tree.child_id) OR names.sort IS NULL
GROUP BY child_id
ORDER BY level, child_id
    )-");
    resetAndLoadData();
}

void AncestorQueryModel::resetAndLoadData() {
    QSqlQuery query;
    query.prepare(query_);
    query.bindValue(QStringLiteral(":person"), person);

    if (!query.exec()) {
        qWarning() << "Something went wrong...";
        qDebug() << query.lastError();
    }

    setQuery(std::move(query));

    setHeaderData(CHILD_ID, Qt::Horizontal, i18n("Child ID"));
    setHeaderData(FATHER_ID, Qt::Horizontal, i18n("Father ID"));
    setHeaderData(MOTHER_ID, Qt::Horizontal, i18n("Mother ID"));
    setHeaderData(VISITED, Qt::Horizontal, i18n("Visited"));
    setHeaderData(LEVEL, Qt::Horizontal, i18n("Level"));
}

ParentQueryModel::ParentQueryModel(IntegerPrimaryKey person, QObject* parent) : QSqlQueryModel(parent), person(person) {
    this->query_ = QStringLiteral(R"-(
SELECT parent_relation.person_id,
       parent_relation.role_id,
       event_roles.role,
       names.titles,
       names.given_names,
       names.prefix,
       names.surname
FROM event_relations AS child_relation
       LEFT JOIN event_relations AS parent_relation ON child_relation.event_id = parent_relation.event_id
       LEFT JOIN event_roles ON parent_relation.role_id = event_roles.id
       LEFT JOIN names ON parent_relation.person_id = names.person_id
WHERE child_relation.person_id = :person
  AND child_relation.role_id = (SELECT id FROM event_roles WHERE role = 'Primary')
  AND event_roles.role IN ('Father', 'Mother')
  AND (names.sort = (SELECT MIN(name2.sort) FROM names AS name2 WHERE name2.person_id = parent_relation.person_id) OR names.sort = NULL)
ORDER BY parent_relation.person_id;
    )-");
    resetAndLoadData();
}

void ParentQueryModel::resetAndLoadData() {
    QSqlQuery query;
    query.prepare(query_);
    query.bindValue(QStringLiteral(":person"), person);

    if (!query.exec()) {
        qWarning() << "Something went wrong...";
        qDebug() << query.lastError();
    }

    setQuery(std::move(query));

    setHeaderData(ROLE_ID, Qt::Horizontal, i18n("Role ID"));
    setHeaderData(ROLE, Qt::Horizontal, i18n("Role"));
    setHeaderData(PERSON_ID, Qt::Horizontal, i18n("Person ID"));
    setHeaderData(GIVEN_NAMES, Qt::Horizontal, i18n("Given names"));
    setHeaderData(PREFIX, Qt::Horizontal, i18n("Prefixes"));
    setHeaderData(SURNAME, Qt::Horizontal, i18n("Surnames"));
}
