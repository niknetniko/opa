CREATE TABLE media (
  id        INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  path      TEXT    NOT NULL,
  title     TEXT,
  note      TEXT,
  mime_type TEXT    NOT NULL
);

CREATE TABLE person_media (
  person_id INTEGER NOT NULL REFERENCES people (id)  ON DELETE CASCADE,
  media_id  INTEGER NOT NULL REFERENCES media  (id)  ON DELETE CASCADE,
  PRIMARY KEY (person_id, media_id)
);

CREATE TABLE name_media (
  name_id  INTEGER NOT NULL REFERENCES names (id) ON DELETE CASCADE,
  media_id INTEGER NOT NULL REFERENCES media (id) ON DELETE CASCADE,
  PRIMARY KEY (name_id, media_id)
);

CREATE TABLE event_media (
  event_id INTEGER NOT NULL REFERENCES events (id) ON DELETE CASCADE,
  media_id INTEGER NOT NULL REFERENCES media  (id) ON DELETE CASCADE,
  PRIMARY KEY (event_id, media_id)
);

CREATE TABLE event_relation_media (
  event_relation_id INTEGER NOT NULL REFERENCES event_relations (id) ON DELETE CASCADE,
  media_id          INTEGER NOT NULL REFERENCES media            (id) ON DELETE CASCADE,
  PRIMARY KEY (event_relation_id, media_id)
);

CREATE TABLE source_media (
  source_id INTEGER NOT NULL REFERENCES sources (id) ON DELETE CASCADE,
  media_id  INTEGER NOT NULL REFERENCES media   (id) ON DELETE CASCADE,
  PRIMARY KEY (source_id, media_id)
);

CREATE TABLE location_media (
  location_id INTEGER NOT NULL REFERENCES locations (id) ON DELETE CASCADE,
  media_id    INTEGER NOT NULL REFERENCES media     (id) ON DELETE CASCADE,
  PRIMARY KEY (location_id, media_id)
)
