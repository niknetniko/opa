CREATE TABLE people
(
    id   INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    root BOOLEAN,
    sex  TEXT
);

CREATE TABLE name_origins
(
    id     INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    origin TEXT
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
    origin_id   INTEGER NULL DEFAULT NULL REFERENCES name_origins (id) ON DELETE SET DEFAULT
);

CREATE TABLE event_types
(
    id   INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    type TEXT
);

CREATE TABLE event_roles
(
    id   INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    role TEXT
);

CREATE TABLE events
(
    id      INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    type_id INTEGER NOT NULL REFERENCES event_types (id) ON DELETE RESTRICT,
    date    TEXT,
    name    TEXT
);

CREATE TABLE event_relations
(
    event_id  INTEGER NOT NULL REFERENCES events (id) ON DELETE CASCADE,
    person_id INTEGER NOT NULL REFERENCES people (id) ON DELETE CASCADE,
    role_id   INTEGER NOT NULL REFERENCES event_roles (id) ON DELETE RESTRICT,
    PRIMARY KEY (event_id, person_id, role_id)
);
