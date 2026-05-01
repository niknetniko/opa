ALTER TABLE media RENAME TO media_old;

CREATE TABLE media (
  id        INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  path      TEXT    NOT NULL,
  title     TEXT,
  note      TEXT,
  mime_type TEXT    NOT NULL
);

INSERT INTO media SELECT id, path, title, note, mime_type FROM media_old;

DROP TABLE media_old
