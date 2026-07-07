CREATE TABLE person_external_ids (
    person_id INTEGER NOT NULL REFERENCES people (id) ON DELETE CASCADE,
    type TEXT NOT NULL,
    external_id TEXT NOT NULL,
    PRIMARY KEY (person_id, external_id)
);

CREATE TABLE location_external_ids (
    location_id INTEGER NOT NULL REFERENCES locations (id) ON DELETE CASCADE,
    type TEXT NOT NULL,
    external_id TEXT NOT NULL,
    PRIMARY KEY (location_id, external_id)
);

CREATE TABLE family_external_ids (
    family_id INTEGER NOT NULL REFERENCES families (id) ON DELETE CASCADE,
    type TEXT NOT NULL,
    external_id TEXT NOT NULL,
    PRIMARY KEY (family_id, external_id)
);

CREATE TABLE event_external_ids (
    event_id INTEGER NOT NULL REFERENCES events (id) ON DELETE CASCADE,
    type TEXT NOT NULL,
    external_id TEXT NOT NULL,
    PRIMARY KEY (event_id, external_id)
);

CREATE TABLE source_external_ids (
    source_id INTEGER NOT NULL REFERENCES sources (id) ON DELETE CASCADE,
    type TEXT NOT NULL,
    external_id TEXT NOT NULL,
    PRIMARY KEY (source_id, external_id)
);

CREATE TABLE media_external_ids (
    media_id INTEGER NOT NULL REFERENCES media (id) ON DELETE CASCADE,
    type TEXT NOT NULL,
    external_id TEXT NOT NULL,
    PRIMARY KEY (media_id, external_id)
);