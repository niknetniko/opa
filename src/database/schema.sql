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
    person_id   INTEGER NOT NULL,
    sort        INTEGER NOT NULL,
    titles      TEXT,
    given_names TEXT,
    prefix      TEXT,
    surname     TEXT,
    origin_id   INTEGER NULL DEFAULT NULL,
    FOREIGN KEY (person_id) REFERENCES people (id),
    FOREIGN KEY (origin_id) REFERENCES name_origins (id)
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
    type_id INTEGER NOT NULL,
    date    TEXT,
    name    TEXT,
    FOREIGN KEY (type_id) REFERENCES event_types (id)
);

CREATE TABLE event_relations
(
    event_id  INTEGER NOT NULL,
    person_id INTEGER NOT NULL,
    role_id   INTEGER NOT NULL,
    PRIMARY KEY (event_id, person_id, role_id),
    FOREIGN KEY (event_id) REFERENCES events (id),
    FOREIGN KEY (person_id) REFERENCES people (id),
    FOREIGN KEY (role_id) REFERENCES event_roles (id)
);
