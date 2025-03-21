CREATE TABLE people
(
  id   INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  root BOOLEAN,
  sex  TEXT
);

CREATE TABLE name_origins
(
  id      INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  origin  TEXT,
  builtin BOOLEAN NOT NULL DEFAULT FALSE
);

CREATE TABLE names
(
  id          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  person_id   INTEGER NOT NULL REFERENCES people (id) ON DELETE CASCADE,
  sort        INTEGER NOT NULL,
  titles      TEXT,
  given_names TEXT,
  prefix      TEXT,
  surname     TEXT,
  note        TEXT,
  origin_id   INTEGER NULL DEFAULT NULL REFERENCES name_origins (id) ON DELETE SET DEFAULT
);

CREATE TABLE event_types
(
  id      INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type    TEXT,
  builtin BOOLEAN NOT NULL DEFAULT FALSE
);

CREATE TABLE event_roles
(
  id      INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  role    TEXT,
  builtin BOOLEAN NOT NULL DEFAULT FALSE
);

CREATE TABLE events
(
  id      INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type_id INTEGER NOT NULL REFERENCES event_types (id) ON DELETE RESTRICT,
  date    TEXT,
  name    TEXT,
  note    TEXT
);

CREATE TABLE event_relations
(
  event_id  INTEGER NOT NULL REFERENCES events (id) ON DELETE CASCADE,
  person_id INTEGER NOT NULL REFERENCES people (id) ON DELETE CASCADE,
  role_id   INTEGER NOT NULL REFERENCES event_roles (id) ON DELETE RESTRICT,
  PRIMARY KEY (event_id, person_id, role_id)
);

CREATE TABLE repositories
(
  id            INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  name          TEXT    NOT NULL,
  homepage      TEXT,
  address       TEXT,
  note          TEXT,
  link_template TEXT
);

CREATE TABLE repository_properties
(
  id        INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  source_id INTEGER REFERENCES repositories (id) ON DELETE CASCADE,
  type      TEXT    NOT NULL,
  value     TEXT
);

CREATE TABLE sources
(
  id          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  title       TEXT    NOT NULL,
  author      TEXT,
  source_date TEXT,
  confidence  TEXT,
  parent_id   INTEGER REFERENCES sources (id) ON DELETE SET NULL
);

CREATE TABLE source_repositories
(
  source_id     INTEGER NOT NULL REFERENCES sources (id) ON DELETE CASCADE,
  repository_id INTEGER NOT NULL REFERENCES repositories (id) ON DELETE CASCADE
);

CREATE TABLE source_properties
(
  id        INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  source_id INTEGER REFERENCES sources (id) ON DELETE CASCADE,
  type      TEXT    NOT NULL,
  value     TEXT
);

CREATE TABLE citations
(
  id          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  source_id   INTEGER REFERENCES sources (id) ON DELETE CASCADE,
  location    TEXT    NOT NULL,
  confidence  TEXT,
  description TEXT,
  source_text TEXT
);

CREATE TABLE citation_properties
(
  id        INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  source_id INTEGER REFERENCES citations (id) ON DELETE CASCADE,
  type      TEXT    NOT NULL,
  value     TEXT
);
