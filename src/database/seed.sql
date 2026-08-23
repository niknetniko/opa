INSERT INTO
  people (id, root, sex)
VALUES
  (1, TRUE, 'Male'),
  (2, FALSE, 'Female'),
  (3, FALSE, 'Male'),
  (4, FALSE, 'Female'),
  (5, FALSE, 'Male'),
  (6, FALSE, 'Female'),
  (7, FALSE, 'Male'),
  (8, FALSE, 'Female'),
  (9, FALSE, 'Male');

INSERT INTO
  names (
    id,
    person_id,
    sort,
    titles,
    given_names,
    prefix,
    surname,
    note
  )
VALUES
  (1, 1, 1, '', 'John', '', 'Doe', ''),
  (2, 2, 1, '', 'Jane', '', 'Doe', ''),
  (3, 3, 1, '', 'Michael', '', 'Doe', ''),
  (4, 4, 1, '', 'Emily', '', 'Smith', ''),
  (5, 5, 1, '', 'William', '', 'Doe', ''),
  (6, 6, 1, '', 'Elizabeth', '', 'Brown', ''),
  (7, 7, 1, '', 'George', '', 'Smith', ''),
  (8, 8, 1, '', 'Mary', '', 'Taylor', ''),
  (9, 9, 1, '', 'Ebenezer', '', 'No name', '');

INSERT INTO
  events (id, type, date, date_sort, name, note)
VALUES
  (
    1,
    'Birth',
    '{"dateModifier":"NONE","dateQuality":"EXACT","dateType":"SINGLE","day":true,"endDay":false,"month":true,"endMonth":false,"proleptic":2451160,"endProleptic":0,"userText":"","year":true,"endYear":false}',
    2451160,
    'Birth of John',
    ''
  ),
  (
    2,
    'Birth',
    '{"dateModifier":"NONE","dateQuality":"EXACT","dateType":"SINGLE","day":true,"endDay":false,"month":true,"endMonth":false,"proleptic":2451160,"endProleptic":0,"userText":"","year":true,"endYear":false}',
    2451160,
    'Birth of Jane',
    ''
  ),
  (
    6,
    'Birth',
    '{"dateModifier":"NONE","dateQuality":"EXACT","dateType":"SINGLE","day":true,"endDay":false,"month":true,"endMonth":false,"proleptic":2451160,"endProleptic":0,"userText":"","year":true,"endYear":false}',
    2451160,
    'Birth of Michael',
    ''
  ),
  (
    7,
    'Birth',
    '{"dateModifier":"NONE","dateQuality":"EXACT","dateType":"SINGLE","day":true,"endDay":false,"month":true,"endMonth":false,"proleptic":2451160,"endProleptic":0,"userText":"","year":true,"endYear":false}',
    2451160,
    'Birth of Emily',
    ''
  ),
  (
    8,
    'Birth',
    '{"dateModifier":"NONE","dateQuality":"EXACT","dateType":"SINGLE","day":true,"endDay":false,"month":true,"endMonth":false,"proleptic":2451160,"endProleptic":0,"userText":"","year":true,"endYear":false}',
    2451160,
    'Birth of William',
    ''
  ),
  (
    9,
    'Birth',
    '{"dateModifier":"NONE","dateQuality":"EXACT","dateType":"SINGLE","day":true,"endDay":false,"month":true,"endMonth":false,"proleptic":2451160,"endProleptic":0,"userText":"","year":true,"endYear":false}',
    2451160,
    'Birth of Elizabeth',
    ''
  ),
  (
    10,
    'Birth',
    '{"dateModifier":"NONE","dateQuality":"EXACT","dateType":"SINGLE","day":true,"endDay":false,"month":true,"endMonth":false,"proleptic":2451160,"endProleptic":0,"userText":"","year":true,"endYear":false}',
    2451160,
    'Birth of George',
    ''
  ),
  (
    11,
    'Birth',
    '{"dateModifier":"NONE","dateQuality":"EXACT","dateType":"SINGLE","day":true,"endDay":false,"month":true,"endMonth":false,"proleptic":2451160,"endProleptic":0,"userText":"","year":true,"endYear":false}',
    2451160,
    'Birth of Mary',
    ''
  ),
  (
    16,
    'Birth',
    '{"dateModifier":"NONE","dateQuality":"EXACT","dateType":"SINGLE","day":true,"endDay":false,"month":true,"endMonth":false,"proleptic":2451160,"endProleptic":0,"userText":"","year":true,"endYear":false}',
    2451160,
    'Birth of Ebenezer',
    ''
  );

INSERT INTO
  event_relations (event_id, person_id, role)
VALUES
  (1, 1, 'Primary'),
  (2, 2, 'Primary'),
  (6, 3, 'Primary'),
  (7, 4, 'Primary'),
  (8, 5, 'Primary'),
  (9, 6, 'Primary'),
  (10, 7, 'Primary'),
  (11, 8, 'Primary'),
  (16, 9, 'Primary');

INSERT INTO
  event_relations (event_id, person_id, role)
VALUES
  (1, 3, 'Father'),
  (1, 4, 'Mother'),
  (2, 3, 'Father'),
  (2, 4, 'Mother'),
  (6, 5, 'Father'),
  (6, 6, 'Mother'),
  (7, 7, 'Father'),
  (7, 8, 'Mother'),
  (9, 9, 'Father'),
  (10, 9, 'Father');

INSERT INTO
  events (id, type, date, date_sort, name, note)
VALUES
  (
    3,
    'Marriage',
    '{"dateModifier":"NONE","dateQuality":"EXACT","dateType":"SINGLE","day":true,"endDay":false,"month":true,"endMonth":false,"proleptic":2451160,"endProleptic":0,"userText":"","year":true,"endYear":false}',
    2451160,
    'Marriage of Michael and Emily',
    ''
  ),
  (
    4,
    'Marriage',
    '{"dateModifier":"NONE","dateQuality":"EXACT","dateType":"SINGLE","day":true,"endDay":false,"month":true,"endMonth":false,"proleptic":2451160,"endProleptic":0,"userText":"","year":true,"endYear":false}',
    2451160,
    'Marriage of William and Elizabeth',
    ''
  ),
  (
    5,
    'Marriage',
    '{"dateModifier":"NONE","dateQuality":"EXACT","dateType":"SINGLE","day":true,"endDay":false,"month":true,"endMonth":false,"proleptic":2451160,"endProleptic":0,"userText":"","year":true,"endYear":false}',
    2451160,
    'Marriage of George and Mary',
    ''
  );

INSERT INTO
  event_relations (event_id, person_id, role)
VALUES
  (3, 3, 'Primary'),
  (3, 4, 'Partner'),
  (4, 5, 'Primary'),
  (4, 6, 'Partner'),
  (5, 7, 'Primary'),
  (5, 8, 'Partner');

INSERT INTO
  events (id, type, date, date_sort, name, note)
VALUES
  (
    12,
    'Death',
    '{"dateModifier":"NONE","dateQuality":"EXACT","dateType":"SINGLE","day":true,"endDay":false,"month":true,"endMonth":false,"proleptic":2451160,"endProleptic":0,"userText":"","year":true,"endYear":false}',
    2451160,
    'Death of William',
    ''
  ),
  (
    13,
    'Death',
    '{"dateModifier":"NONE","dateQuality":"EXACT","dateType":"SINGLE","day":true,"endDay":false,"month":true,"endMonth":false,"proleptic":2451160,"endProleptic":0,"userText":"","year":true,"endYear":false}',
    2451160,
    'Death of Elizabeth',
    ''
  ),
  (
    14,
    'Death',
    '{"dateModifier":"NONE","dateQuality":"EXACT","dateType":"SINGLE","day":true,"endDay":false,"month":true,"endMonth":false,"proleptic":2451160,"endProleptic":0,"userText":"","year":true,"endYear":false}',
    2451160,
    'Death of George',
    ''
  ),
  (
    15,
    'Death',
    '{"dateModifier":"NONE","dateQuality":"EXACT","dateType":"SINGLE","day":true,"endDay":false,"month":true,"endMonth":false,"proleptic":2451160,"endProleptic":0,"userText":"","year":true,"endYear":false}',
    2451160,
    'Death of Mary',
    ''
  );

INSERT INTO
  event_relations (event_id, person_id, role)
VALUES
  (12, 5, 'Primary'),
  (13, 6, 'Primary'),
  (14, 7, 'Primary'),
  (15, 8, 'Primary');

INSERT INTO
  families (id)
VALUES
  (1),
  (2),
  (3),
  (4);

UPDATE events
SET
  family_id = 1
WHERE
  id IN (1, 2, 3);

UPDATE events
SET
  family_id = 2
WHERE
  id IN (4, 6);

UPDATE events
SET
  family_id = 3
WHERE
  id IN (5, 7);

UPDATE events
SET
  family_id = 4
WHERE
  id IN (9, 10);

INSERT INTO
  sources (
    title,
    type,
    author,
    publication,
    confidence,
    note,
    parent_id
  )
VALUES
  (
    'Smith Family Papers',
    'Register',
    'Archivist, Local Historical Society',
    'Local Historical Society Archives',
    'Normal',
    'Describes a collection of letters, diaries, and legal documents related to the Smith family.',
    NULL
  ),
  (
    'Letter from Mary Smith to John Smith Sr.',
    'Letter',
    'Mary Smith',
    'Smith Family Papers (Collection)',
    'High',
    'Dated July 10, 1870. Mentions the upcoming marriage of "our John" and "dear Jane."',
    1
  ),
  (
    'Marriage Mention in Letter',
    'Letter',
    'Mary Smith',
    'Letter from Mary Smith to John Smith Sr., Page 2, Line 5',
    'High',
    'Exact quote: "...we are all so excited for John and Jane''s wedding next week."',
    2
  ),
  (
    'Birth Certificate of John Doe',
    'Certificate',
    'County Clerk''s Office',
    'County Records, Jamestown, USA',
    'High',
    'Original document, certified copy.',
    NULL
  ),
  (
    'John Doe Birth Date',
    'Certificate',
    'County Clerk',
    'Birth Certificate of John Doe, Entry 456',
    'High',
    'Date of Birth: January 15, 1872',
    4
  ),
  (
    'Father''s Name: John Doe Sr.',
    'Certificate',
    'County Clerk',
    'Birth Certificate of John Doe, Entry 456',
    'High',
    'Father''s name listed as John Doe Sr.',
    4
  ),
  (
    'Mother''s Name: Mary Smith',
    'Certificate',
    'County Clerk',
    'Birth Certificate of John Doe, Entry 456',
    'High',
    'Mother''s maiden name listed as Mary Smith',
    4
  ),
  (
    'Marriage Record of John Doe and Jane Smith',
    'Certificate',
    'County Clerk''s Office',
    'County Records, Jamestown, USA',
    'High',
    'Original document, certified copy.',
    NULL
  ),
  (
    'Marriage Date: July 25, 1895',
    'Certificate',
    'County Clerk',
    'Marriage Record of John Doe and Jane Smith, Entry 123',
    'High',
    'Date of Marriage: July 25, 1895',
    8
  ),
  (
    '1900 US Census, Jamestown, Jamesstate',
    'Census',
    'US Census Bureau',
    'National Archives and Records Administration',
    'Normal',
    'Information provided by head of household. Potential for errors.',
    NULL
  ),
  (
    'John Doe Household, 1900 Census',
    'Census',
    'US Census Bureau',
    '1900 US Census, Jamestown, Jamesstate, ED 12, Sheet 5, Line 23',
    'Normal',
    'Lists John Doe, age 28, occupation: Farmer',
    10
  ),
  (
    'Jane Doe (Wife), 1900 Census',
    'Census',
    'US Census Bureau',
    '1900 US Census, Jamestown, Jamesstate, ED 12, Sheet 5, Line 24',
    'Normal',
    'Lists Jane Doe, age 26, wife',
    10
  ),
  (
    'Death Certificate of Mary Smith (nee Brown)',
    'Certificate',
    'State Department of Health',
    'State Vital Records, Jamesstate, USA',
    'High',
    'Original document, certified copy.  Indicates maiden name.',
    NULL
  ),
  (
    'Mary Smith Date of Death',
    'Certificate',
    'State Department of Health',
    'Death Certificate of Mary Smith, Certificate Number 12345',
    'High',
    'Date of Death: March 10, 1910',
    13
  ),
  (
    'Mary Smith Place of Birth',
    'Certificate',
    'State Department of Health',
    'Death Certificate of Mary Smith, Certificate Number 12345',
    'Normal',
    'Place of Birth: Jamestown, Jamesstate',
    13
  ),
  (
    'Abstract of Will for John Doe Sr.',
    'Will',
    'County Clerk''s Office',
    'Will Book 4, Page 123',
    'Normal',
    'Summarized record, contains key information, but potential omissions',
    NULL
  ),
  (
    'Mention of Wife, Mary',
    'Will',
    'County Clerk',
    'Abstract of Will for John Doe Sr., Page 123, line 10',
    'High',
    'Will proved on June, 2nd 1875, mentions wife Mary Doe',
    16
  ),
  (
    'Mention of Son, John',
    'Will',
    'County Clerk',
    'Abstract of Will for John Doe Sr., Page 124, line 1',
    'High',
    'Mentions son, John Doe',
    16
  ),
  (
    'Birth Certificate of Jane Smith',
    'Certificate',
    'County Clerk''s Office',
    'County Records, Jamestown, USA',
    'High',
    'Original document, certified copy.',
    NULL
  ),
  (
    'Father: Robert Smith',
    'Certificate',
    'County Clerk',
    'Birth Certificate of Jane Smith, Entry 789',
    'High',
    'Father''s name listed as Robert Smith',
    19
  ),
  (
    'Mother: Emily Green',
    'Certificate',
    'County Clerk',
    'Birth Certificate of Jane Smith, Entry 789',
    'High',
    'Mother''s maiden name listed as Emily Green',
    19
  );
