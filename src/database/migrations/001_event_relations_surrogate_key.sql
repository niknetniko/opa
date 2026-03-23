CREATE TABLE event_relations_new (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  event_id INTEGER NOT NULL REFERENCES events (id) ON DELETE CASCADE,
  person_id INTEGER NOT NULL REFERENCES people (id) ON DELETE CASCADE,
  role_id INTEGER NOT NULL REFERENCES event_roles (id) ON DELETE RESTRICT,
  UNIQUE (event_id, person_id, role_id)
);

INSERT INTO event_relations_new (event_id, person_id, role_id)
  SELECT event_id, person_id, role_id FROM event_relations;

CREATE TABLE event_relation_citations_new (
  event_relation_id INTEGER NOT NULL REFERENCES event_relations_new (id) ON DELETE CASCADE,
  source_id INTEGER NOT NULL REFERENCES sources (id) ON DELETE CASCADE,
  PRIMARY KEY (event_relation_id, source_id)
);

INSERT INTO event_relation_citations_new (event_relation_id, source_id)
  SELECT ern.id, erc.source_id
  FROM event_relation_citations erc
  JOIN event_relations_new ern
    ON ern.event_id = erc.event_id
   AND ern.person_id = erc.person_id
   AND ern.role_id = erc.role_id;

DROP TABLE event_relation_citations;
DROP TABLE event_relations;
ALTER TABLE event_relations_new RENAME TO event_relations;
ALTER TABLE event_relation_citations_new RENAME TO event_relation_citations
