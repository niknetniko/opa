CREATE TABLE IF NOT EXISTS people
(
    id   INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    root BOOLEAN,
    sex  TEXT
);

CREATE TABLE IF NOT EXISTS names
(
    id          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    person_id   INTEGER NOT NULL,
    given_names TEXT,
    titles      TEXT,
    nickname    TEXT,
    prefix      TEXT,
    surname     TEXT,
    origin      TEXT,
    sort_as     TEXT,
    main        BOOLEAN,
    FOREIGN KEY (person_id) REFERENCES people (id)
);

CREATE TABLE IF NOT EXISTS events
(
    id   INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    type TEXT    NOT NULL,
    date TEXT
);

CREATE TABLE IF NOT EXISTS event_people
(
    person_id INTEGER,
    event_id  INTEGER,
    role      TEXT,
    FOREIGN KEY (person_id) REFERENCES people (id),
    FOREIGN KEY (event_id) REFERENCES events (id)
);

CREATE TABLE IF NOT EXISTS sources
(
    id     INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    title  TEXT    NOT NULL,
    author TEXT,
    date   TEXT
);

CREATE TABLE IF NOT EXISTS citations
(
    id        INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    source_id INTEGER,
    part      TEXT,
    FOREIGN KEY (source_id) REFERENCES sources (id)
);

CREATE TABLE IF NOT EXISTS event_people_citations
(
    person_id   INTEGER,
    event_id    INTEGER,
    citation_id INTEGER,
    FOREIGN KEY (person_id, event_id) REFERENCES event_people (person_id, event_id),
    FOREIGN KEY (citation_id) REFERENCES citations (id)
);

CREATE TABLE IF NOT EXISTS event_citations
(
    event_id    INTEGER,
    citation_id INTEGER,
    FOREIGN KEY (event_id) REFERENCES events (id),
    FOREIGN KEY (citation_id) REFERENCES citations (id)
);

CREATE TABLE IF NOT EXISTS name_citations
(
    name_id     INTEGER,
    citation_id INTEGER,
    FOREIGN KEY (name_id) REFERENCES names (id),
    FOREIGN KEY (citation_id) REFERENCES citations (id)
);

CREATE TABLE IF NOT EXISTS person_citations
(
    person_id   INTEGER,
    citation_id INTEGER,
    FOREIGN KEY (person_id) REFERENCES people (id),
    FOREIGN KEY (citation_id) REFERENCES citations (id)
);
