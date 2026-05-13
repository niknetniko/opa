/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "./family_repository.h"

#include "../../core/data_event_broker.h"

#include <QSqlQuery>

using namespace Qt::StringLiterals;

static const auto FAMILY_MEMBERS_SQL = QStringLiteral(R"-(
WITH my_families AS (
    SELECT DISTINCT e.family_id
    FROM events e
    JOIN event_relations er ON e.id = er.event_id
    JOIN event_roles r ON er.role_id = r.id
    JOIN event_types et ON e.type_id = et.id
    WHERE er.person_id = :id
      AND r.role IN ('Father', 'Mother')
      AND et.type = 'Birth'
      AND e.family_id IS NOT NULL
)
SELECT et.type           AS event_type,
       e.type_id         AS event_type_id,
       er.person_id      AS person_id,
       NULL              AS partner_id,
       e.id              AS event_id,
       e.date            AS event_date,
       e.date_sort       AS event_date_sort,
       e.family_id       AS family_id,
       names.titles      AS titles,
       names.given_names AS given_names,
       names.prefix      AS prefix,
       names.surname     AS surname
FROM events e
JOIN event_types et ON e.type_id = et.id
JOIN event_relations er ON e.id = er.event_id
JOIN event_roles r ON er.role_id = r.id
LEFT JOIN names ON er.person_id = names.person_id
WHERE e.family_id IN (SELECT family_id FROM my_families)
  AND (
      (et.type = 'Birth' AND r.role = 'Primary')
      OR (et.type = 'Marriage' AND r.role IN ('Primary', 'Partner') AND er.person_id != :id)
  )
  AND (names.sort = (SELECT MIN(n2.sort) FROM names AS n2 WHERE n2.person_id = er.person_id) OR names.sort IS NULL)
ORDER BY event_type, event_date_sort ASC NULLS LAST;
)-");

static const auto ANCESTORS_SQL = QStringLiteral(R"-(
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

     SELECT er.person_id AS child_id, NULL AS father_id, NULL AS mother_id
     FROM event_relations er
            JOIN event_roles ON er.role_id = event_roles.id
     WHERE event_roles.role IN ('Father', 'Mother')
       AND er.person_id NOT IN (SELECT person_id
                                FROM event_relations
                                WHERE role_id = (SELECT id FROM event_roles WHERE role = 'Primary'))
     GROUP BY er.person_id

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

static const auto FAMILIES_OVERVIEW_SQL = QStringLiteral(R"-(
WITH parent_surnames AS (
    SELECT e.family_id, er.person_id, n.surname
    FROM events e
    JOIN event_types et ON e.type_id = et.id
    JOIN event_relations er ON e.id = er.event_id
    JOIN event_roles r ON er.role_id = r.id
    JOIN names n ON er.person_id = n.person_id
    WHERE et.type = 'Birth'
      AND r.role IN ('Father', 'Mother')
      AND (n.sort = (SELECT MIN(n2.sort) FROM names n2 WHERE n2.person_id = er.person_id) OR n.sort IS NULL)
    GROUP BY e.family_id, er.person_id
),
parent_names AS (
    SELECT family_id, GROUP_CONCAT(surname, ' — ') AS display_name
    FROM (SELECT family_id, person_id, surname FROM parent_surnames ORDER BY family_id, person_id)
    GROUP BY family_id
)
SELECT f.id                                           AS family_id,
       COALESCE(pn.display_name, 'Family #' || f.id) AS family_display_name,
       e.id                                           AS event_id,
       et.type                                        AS event_type,
       e.date                                         AS event_date,
       e.date_sort                                    AS event_date_sort,
       er.person_id                                   AS person_id,
       r.role                                         AS role,
       n.titles                                       AS titles,
       n.given_names                                  AS given_names,
       n.prefix                                       AS prefix,
       n.surname                                      AS surname
FROM families f
LEFT JOIN parent_names pn ON f.id = pn.family_id
JOIN events e ON e.family_id = f.id
JOIN event_types et ON e.type_id = et.id
JOIN event_relations er ON e.id = er.event_id
JOIN event_roles r ON er.role_id = r.id
LEFT JOIN names n ON er.person_id = n.person_id
WHERE ((et.type = 'Birth' AND r.role = 'Primary')
    OR (et.type = 'Marriage' AND r.role IN ('Primary', 'Partner')))
  AND (n.sort = (SELECT MIN(n2.sort) FROM names n2 WHERE n2.person_id = er.person_id) OR n.sort IS NULL)
ORDER BY f.id,
         CASE WHEN et.type = 'Marriage' THEN 0 ELSE 1 END,
         e.date_sort ASC NULLS LAST
)-");

static const auto PARENTS_SQL = QStringLiteral(R"-(
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

QList<FamilyOverviewRow> FamilyRepository::findAllFamiliesOverview() const {
    return fetchAll<FamilyOverviewRow>(FAMILIES_OVERVIEW_SQL, {});
}

QList<FamilyMemberEntity> FamilyRepository::findFamilyMembersForPerson(IntegerPrimaryKey personId) const {
    return fetchAll<FamilyMemberEntity>(FAMILY_MEMBERS_SQL, {{u":id"_s, personId}});
}

QList<AncestorEntity> FamilyRepository::findAncestorsForPerson(IntegerPrimaryKey personId) const {
    return fetchAll<AncestorEntity>(ANCESTORS_SQL, {{u":person"_s, personId}});
}

QList<ParentEntity> FamilyRepository::findParentsForPerson(IntegerPrimaryKey personId) const {
    return fetchAll<ParentEntity>(PARENTS_SQL, {{u":person"_s, personId}});
}

std::optional<IntegerPrimaryKey> FamilyRepository::createFamily() {
    QSqlQuery query;
    if (!query.exec(u"INSERT INTO families DEFAULT VALUES"_s)) {
        return std::nullopt;
    }
    auto id = query.lastInsertId().toLongLong();
    DataEventBroker::instance().notifyChanged<Schema::Families>(id);
    return id;
}

bool FamilyRepository::linkEventToFamily(IntegerPrimaryKey eventId, IntegerPrimaryKey familyId) {
    QSqlQuery query;
    query.prepare(u"UPDATE events SET family_id = :fid WHERE id = :eid"_s);
    query.bindValue(u":fid"_s, familyId);
    query.bindValue(u":eid"_s, eventId);
    if (!query.exec()) {
        return false;
    }
    DataEventBroker::instance().notifyChanged<Schema::Events>(eventId);
    return true;
}
