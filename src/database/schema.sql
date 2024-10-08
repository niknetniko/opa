CREATE TABLE people
(
    id   INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    root BOOLEAN,
    sex  TEXT
);

CREATE TABLE name_origins
(
    id     INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    origin TEXT UNIQUE
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
