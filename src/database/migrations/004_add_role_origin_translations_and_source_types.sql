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
  id      INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type    TEXT NOT NULL,
  builtin BOOLEAN NOT NULL DEFAULT FALSE
);

CREATE TABLE source_type_translations (
  id      INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type_id INTEGER NOT NULL REFERENCES source_types (id) ON DELETE CASCADE,
  locale  TEXT NOT NULL,
  name    TEXT NOT NULL,
  UNIQUE (type_id, locale)
);

INSERT INTO source_types (type, builtin)
  VALUES
    ('Certificate', true),
    ('Register', true),
    ('Census', true),
    ('Will', true),
    ('Letter', true),
    ('Newspaper', true),
    ('Book', true),
    ('Website', true),
    ('Photograph', true);

INSERT INTO source_types (type, builtin)
  SELECT DISTINCT type, FALSE FROM sources WHERE type IS NOT NULL AND type NOT IN (SELECT type FROM source_types);

ALTER TABLE sources ADD COLUMN type_id INTEGER REFERENCES source_types (id) ON DELETE SET NULL;

UPDATE sources SET type_id = (SELECT id FROM source_types WHERE type = sources.type) WHERE type IS NOT NULL;

ALTER TABLE sources DROP COLUMN type;
