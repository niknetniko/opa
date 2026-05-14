DROP TABLE IF EXISTS _fam_map;

CREATE TEMP TABLE _fam_map (
  p1 INTEGER NOT NULL,
  p2 INTEGER NOT NULL,
  family_id INTEGER
);

INSERT INTO
  _fam_map (p1, p2)
SELECT DISTINCT
  p1,
  p2
FROM
  (
    SELECT
      MIN(er.person_id) AS p1,
      MAX(er.person_id) AS p2
    FROM
      events e
      JOIN event_types et ON e.type_id = et.id
      JOIN event_relations er ON e.id = er.event_id
      JOIN event_roles r ON er.role_id = r.id
    WHERE
      et.type = 'Birth'
      AND r.role IN ('Father', 'Mother')
      AND e.family_id IS NULL
    GROUP BY
      e.id
  );

INSERT INTO
  families (note)
SELECT
  CAST(p1 AS TEXT) || ',' || CAST(p2 AS TEXT)
FROM
  _fam_map
ORDER BY
  p1,
  p2;

UPDATE _fam_map
SET
  family_id = (
    SELECT
      f.id
    FROM
      families f
    WHERE
      f.note = CAST(_fam_map.p1 AS TEXT) || ',' || CAST(_fam_map.p2 AS TEXT)
  );

UPDATE events
SET
  family_id = (
    SELECT
      m.family_id
    FROM
      _fam_map m
    WHERE
      m.p1 = (
        SELECT
          MIN(er.person_id)
        FROM
          event_relations er
          JOIN event_roles r ON er.role_id = r.id
        WHERE
          er.event_id = events.id
          AND r.role IN ('Father', 'Mother')
      )
      AND m.p2 = (
        SELECT
          MAX(er.person_id)
        FROM
          event_relations er
          JOIN event_roles r ON er.role_id = r.id
        WHERE
          er.event_id = events.id
          AND r.role IN ('Father', 'Mother')
      )
  )
WHERE
  family_id IS NULL
  AND id IN (
    SELECT
      e.id
    FROM
      events e
      JOIN event_types et ON e.type_id = et.id
    WHERE
      et.type = 'Birth'
  );

UPDATE events
SET
  family_id = (
    SELECT
      m.family_id
    FROM
      _fam_map m
    WHERE
      m.p1 = (
        SELECT
          MIN(er.person_id)
        FROM
          event_relations er
          JOIN event_roles r ON er.role_id = r.id
        WHERE
          er.event_id = events.id
          AND r.role IN ('Primary', 'Partner')
      )
      AND m.p2 = (
        SELECT
          MAX(er.person_id)
        FROM
          event_relations er
          JOIN event_roles r ON er.role_id = r.id
        WHERE
          er.event_id = events.id
          AND r.role IN ('Primary', 'Partner')
      )
  )
WHERE
  family_id IS NULL
  AND id IN (
    SELECT
      e.id
    FROM
      events e
      JOIN event_types et ON e.type_id = et.id
    WHERE
      et.type = 'Marriage'
  )
  AND (
    SELECT
      COUNT(DISTINCT e2.id)
    FROM
      events e2
      JOIN event_types et2 ON e2.type_id = et2.id
    WHERE
      et2.type = 'Marriage'
      AND (
        SELECT
          MIN(er2.person_id)
        FROM
          event_relations er2
          JOIN event_roles r2 ON er2.role_id = r2.id
        WHERE
          er2.event_id = e2.id
          AND r2.role IN ('Primary', 'Partner')
      ) = (
        SELECT
          MIN(er.person_id)
        FROM
          event_relations er
          JOIN event_roles r ON er.role_id = r.id
        WHERE
          er.event_id = events.id
          AND r.role IN ('Primary', 'Partner')
      )
      AND (
        SELECT
          MAX(er2.person_id)
        FROM
          event_relations er2
          JOIN event_roles r2 ON er2.role_id = r2.id
        WHERE
          er2.event_id = e2.id
          AND r2.role IN ('Primary', 'Partner')
      ) = (
        SELECT
          MAX(er.person_id)
        FROM
          event_relations er
          JOIN event_roles r ON er.role_id = r.id
        WHERE
          er.event_id = events.id
          AND r.role IN ('Primary', 'Partner')
      )
  ) = 1;

UPDATE families
SET
  note = NULL
WHERE
  id IN (
    SELECT
      family_id
    FROM
      _fam_map
  );

DROP TABLE _fam_map
