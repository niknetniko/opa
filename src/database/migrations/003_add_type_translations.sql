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
)
