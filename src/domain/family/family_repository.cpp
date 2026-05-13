/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "./family_repository.h"

using namespace Qt::StringLiterals;

static const auto FAMILY_MEMBERS_SQL = QStringLiteral(R"-(
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
               events.date_sort         AS birth_date_sort,
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
               events.date_sort          AS marriage_date_sort,
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
       child.birth_event_id  AS event_id,
       child.birth_date      AS event_date,
       child.birth_date_sort AS event_date_sort,
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
       marriage.marriage_event_id  AS event_id,
       marriage.marriage_date      AS event_date,
       marriage.marriage_date_sort AS event_date_sort,
       names.titles               AS titles,
       names.given_names          AS given_names,
       names.prefix               AS prefix,
       names.surname              AS surname
FROM relationships AS marriage
       LEFT JOIN names ON marriage.partner_id = names.person_id
WHERE names.sort = (SELECT MIN(n2.sort) FROM names AS n2 WHERE n2.person_id = marriage.partner_id) OR names.sort = NULL

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

QList<FamilyMemberEntity> FamilyRepository::findFamilyMembersForPerson(IntegerPrimaryKey personId) const {
    return fetchAll<FamilyMemberEntity>(FAMILY_MEMBERS_SQL, {{u":id"_s, personId}});
}

QList<AncestorEntity> FamilyRepository::findAncestorsForPerson(IntegerPrimaryKey personId) const {
    return fetchAll<AncestorEntity>(ANCESTORS_SQL, {{u":person"_s, personId}});
}

QList<ParentEntity> FamilyRepository::findParentsForPerson(IntegerPrimaryKey personId) const {
    return fetchAll<ParentEntity>(PARENTS_SQL, {{u":person"_s, personId}});
}
