ALTER TABLE events ADD COLUMN date_sort INTEGER;
UPDATE events
   SET date_sort = json_extract(date, '$.proleptic')
 WHERE date IS NOT NULL AND LENGTH(date) > 0;

ALTER TABLE locations ADD COLUMN date_start_sort INTEGER;
UPDATE locations
   SET date_start_sort = json_extract(date_start, '$.proleptic')
 WHERE date_start IS NOT NULL AND LENGTH(date_start) > 0;

ALTER TABLE locations ADD COLUMN date_end_sort INTEGER;
UPDATE locations
   SET date_end_sort = json_extract(date_end, '$.proleptic')
 WHERE date_end IS NOT NULL AND LENGTH(date_end) > 0;
