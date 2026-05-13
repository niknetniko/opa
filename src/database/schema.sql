CREATE TABLE people (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  root BOOLEAN,
  sex TEXT
);

CREATE TABLE name_origins (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  origin TEXT,
  builtin BOOLEAN NOT NULL DEFAULT FALSE
);

CREATE TABLE names (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  person_id INTEGER NOT NULL REFERENCES people (id) ON DELETE CASCADE,
  sort INTEGER NOT NULL,
  titles TEXT,
  given_names TEXT,
  prefix TEXT,
  surname TEXT,
  note TEXT,
  origin_id INTEGER NULL DEFAULT NULL REFERENCES name_origins (id) ON DELETE SET DEFAULT
);

CREATE TABLE event_types (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type TEXT,
  builtin BOOLEAN NOT NULL DEFAULT FALSE
);

CREATE TABLE event_roles (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  role TEXT,
  builtin BOOLEAN NOT NULL DEFAULT FALSE
);

CREATE TABLE location_types (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type TEXT NOT NULL,
  builtin BOOLEAN NOT NULL DEFAULT FALSE
);

CREATE TABLE locations (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL,
  type_id INTEGER NULL REFERENCES location_types (id) ON DELETE SET NULL,
  parent_id INTEGER NULL REFERENCES locations (id) ON DELETE SET NULL,
  note TEXT,
  latitude REAL,
  longitude REAL,
  date_start TEXT,
  date_start_sort INTEGER,
  date_end TEXT,
  date_end_sort INTEGER
);

CREATE TABLE families (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  note TEXT
);

CREATE TABLE events (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type_id INTEGER NOT NULL REFERENCES event_types (id) ON DELETE RESTRICT,
  date TEXT,
  date_sort INTEGER,
  name TEXT,
  note TEXT,
  location_id INTEGER NULL REFERENCES locations (id) ON DELETE SET NULL,
  family_id INTEGER NULL REFERENCES families (id) ON DELETE SET NULL
);

CREATE TABLE event_relations (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  event_id INTEGER NOT NULL REFERENCES events (id) ON DELETE CASCADE,
  person_id INTEGER NOT NULL REFERENCES people (id) ON DELETE CASCADE,
  role_id INTEGER NOT NULL REFERENCES event_roles (id) ON DELETE RESTRICT,
  UNIQUE (event_id, person_id, role_id)
);

CREATE TABLE sources (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  title TEXT,
  type_id INTEGER NULL REFERENCES source_types (id) ON DELETE SET NULL,
  author TEXT,
  publication TEXT,
  confidence TEXT NOT NULL,
  note TEXT,
  parent_id INTEGER REFERENCES sources (id) ON DELETE SET NULL
);

CREATE TABLE event_citations (
  event_id INTEGER NOT NULL REFERENCES events (id) ON DELETE CASCADE,
  source_id INTEGER NOT NULL REFERENCES sources (id) ON DELETE CASCADE,
  PRIMARY KEY (event_id, source_id)
);

CREATE TABLE event_relation_citations (
  event_relation_id INTEGER NOT NULL REFERENCES event_relations (id) ON DELETE CASCADE,
  source_id INTEGER NOT NULL REFERENCES sources (id) ON DELETE CASCADE,
  PRIMARY KEY (event_relation_id, source_id)
);

CREATE TABLE name_citations (
  name_id INTEGER NOT NULL REFERENCES names (id) ON DELETE CASCADE,
  source_id INTEGER NOT NULL REFERENCES sources (id) ON DELETE CASCADE,
  PRIMARY KEY (name_id, source_id)
);

CREATE TABLE person_citations (
  person_id INTEGER NOT NULL REFERENCES people (id) ON DELETE CASCADE,
  source_id INTEGER NOT NULL REFERENCES sources (id) ON DELETE CASCADE,
  PRIMARY KEY (person_id, source_id)
);

CREATE TABLE event_role_translations (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  role_id INTEGER NOT NULL REFERENCES event_roles (id) ON DELETE CASCADE,
  locale TEXT NOT NULL,
  name TEXT NOT NULL,
  UNIQUE (role_id, locale)
);

CREATE TABLE name_origin_translations (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  origin_id INTEGER NOT NULL REFERENCES name_origins (id) ON DELETE CASCADE,
  locale TEXT NOT NULL,
  name TEXT NOT NULL,
  UNIQUE (origin_id, locale)
);

CREATE TABLE source_types (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type TEXT NOT NULL,
  builtin BOOLEAN NOT NULL DEFAULT FALSE
);

CREATE TABLE source_type_translations (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type_id INTEGER NOT NULL REFERENCES source_types (id) ON DELETE CASCADE,
  locale TEXT NOT NULL,
  name TEXT NOT NULL,
  UNIQUE (type_id, locale)
);

CREATE TABLE event_type_translations (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type_id INTEGER NOT NULL REFERENCES event_types (id) ON DELETE CASCADE,
  locale TEXT NOT NULL,
  name TEXT NOT NULL,
  UNIQUE (type_id, locale)
);

CREATE TABLE location_type_translations (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type_id INTEGER NOT NULL REFERENCES location_types (id) ON DELETE CASCADE,
  locale TEXT NOT NULL,
  name TEXT NOT NULL,
  UNIQUE (type_id, locale)
);

CREATE TABLE media (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  path TEXT NOT NULL,
  title TEXT,
  note TEXT,
  mime_type TEXT NOT NULL
);

CREATE TABLE person_media (
  person_id INTEGER NOT NULL REFERENCES people (id) ON DELETE CASCADE,
  media_id INTEGER NOT NULL REFERENCES media (id) ON DELETE CASCADE,
  PRIMARY KEY (person_id, media_id)
);

CREATE TABLE name_media (
  name_id INTEGER NOT NULL REFERENCES names (id) ON DELETE CASCADE,
  media_id INTEGER NOT NULL REFERENCES media (id) ON DELETE CASCADE,
  PRIMARY KEY (name_id, media_id)
);

CREATE TABLE event_media (
  event_id INTEGER NOT NULL REFERENCES events (id) ON DELETE CASCADE,
  media_id INTEGER NOT NULL REFERENCES media (id) ON DELETE CASCADE,
  PRIMARY KEY (event_id, media_id)
);

CREATE TABLE event_relation_media (
  event_relation_id INTEGER NOT NULL REFERENCES event_relations (id) ON DELETE CASCADE,
  media_id INTEGER NOT NULL REFERENCES media (id) ON DELETE CASCADE,
  PRIMARY KEY (event_relation_id, media_id)
);

CREATE TABLE source_media (
  source_id INTEGER NOT NULL REFERENCES sources (id) ON DELETE CASCADE,
  media_id INTEGER NOT NULL REFERENCES media (id) ON DELETE CASCADE,
  PRIMARY KEY (source_id, media_id)
);

CREATE TABLE location_media (
  location_id INTEGER NOT NULL REFERENCES locations (id) ON DELETE CASCADE,
  media_id INTEGER NOT NULL REFERENCES media (id) ON DELETE CASCADE,
  PRIMARY KEY (location_id, media_id)
);
