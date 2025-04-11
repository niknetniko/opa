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
    note,
    origin_id
  )
VALUES
  (1, 1, 1, '', 'John', '', 'Doe', '', 1),
  (2, 2, 1, '', 'Jane', '', 'Doe', '', 1),
  (3, 3, 1, '', 'Michael', '', 'Doe', '', 1),
  (4, 4, 1, '', 'Emily', '', 'Smith', '', 1),
  (5, 5, 1, '', 'William', '', 'Doe', '', 1),
  (6, 6, 1, '', 'Elizabeth', '', 'Brown', '', 1),
  (7, 7, 1, '', 'George', '', 'Smith', '', 1),
  (8, 8, 1, '', 'Mary', '', 'Taylor', '', 1),
  (9, 9, 1, '', 'Ebenezer', '', 'No name', '', 1);

INSERT INTO
  events (id, type_id, date, name, note)
VALUES
  (
    1,
    1,
    '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
    'Birth of John',
    ''
  ),
  (
    2,
    1,
    '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
    'Birth of Jane',
    ''
  ),
  (
    6,
    1,
    '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
    'Birth of Michael',
    ''
  ),
  (
    7,
    1,
    '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
    'Birth of Emily',
    ''
  ),
  (
    8,
    1,
    '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
    'Birth of William',
    ''
  ),
  (
    9,
    1,
    '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
    'Birth of Elizabeth',
    ''
  ),
  (
    10,
    1,
    '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
    'Birth of George',
    ''
  ),
  (
    11,
    1,
    '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
    'Birth of Mary',
    ''
  ),
  (
    16,
    1,
    '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
    'Birth of Ebenezer',
    ''
  );

INSERT INTO
  event_relations (event_id, person_id, role_id)
VALUES
  (1, 1, 1),
  (2, 2, 1),
  (6, 3, 1),
  (7, 4, 1),
  (8, 5, 1),
  (9, 6, 1),
  (10, 7, 1),
  (11, 8, 1),
  (16, 9, 1);

INSERT INTO
  event_relations (event_id, person_id, role_id)
VALUES
  (1, 3, 5),
  (1, 4, 4),
  (2, 3, 5),
  (2, 4, 4),
  (6, 5, 5),
  (6, 6, 4),
  (7, 7, 5),
  (7, 8, 4),
  (9, 9, 5),
  (10, 9, 5);

INSERT INTO
  events (id, type_id, date, name, note)
VALUES
  (
    3,
    3,
    '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
    'Marriage of Michael and Emily',
    ''
  ),
  (
    4,
    3,
    '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
    'Marriage of William and Elizabeth',
    ''
  ),
  (
    5,
    3,
    '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
    'Marriage of George and Mary',
    ''
  );

INSERT INTO
  event_relations (event_id, person_id, role_id)
VALUES
  (3, 3, 1),
  (3, 4, 2),
  (4, 5, 1),
  (4, 6, 2),
  (5, 7, 1),
  (5, 8, 2);

INSERT INTO
  events (id, type_id, date, name, note)
VALUES
  (
    12,
    2,
    '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
    'Death of William',
    ''
  ),
  (
    13,
    2,
    '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
    'Death of Elizabeth',
    ''
  ),
  (
    14,
    2,
    '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
    'Death of George',
    ''
  ),
  (
    15,
    2,
    '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
    'Death of Mary',
    ''
  );

INSERT INTO
  event_relations (event_id, person_id, role_id)
VALUES
  (12, 5, 1),
  (13, 6, 1),
  (14, 7, 1),
  (15, 8, 1);

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
    'Collection Abstract',
    'Archivist, Local Historical Society',
    'Local Historical Society Archives',
    'Medium',
    'Describes a collection of letters, diaries, and legal documents related to the Smith family.',
    NULL
  );

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
    'Letter from Mary Smith to John Smith Sr.',
    'Letter',
    'Mary Smith',
    'Smith Family Papers (Collection)',
    'High',
    'Dated July 10, 1870. Mentions the upcoming marriage of "our John" and "dear Jane."',
    1
  );

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
    'Marriage Mention in Letter',
    'Citation',
    'Mary Smith',
    'Letter from Mary Smith to John Smith Sr., Page 2, Line 5',
    'High',
    'Exact quote: "...we are all so excited for John and Jane''s wedding next week."',
    2
  );

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
    'Birth Certificate of John Doe',
    'Birth Certificate',
    'County Clerk''s Office',
    'County Records, Jamestown, USA',
    'High',
    'Original document, certified copy.',
    NULL
  );

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
    'John Doe Birth Date',
    'Citation',
    'County Clerk',
    'Birth Certificate of John Doe, Entry 456',
    'High',
    'Date of Birth: January 15, 1872',
    4
  );

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
    'Father''s Name: John Doe Sr.',
    'Citation',
    'County Clerk',
    'Birth Certificate of John Doe, Entry 456',
    'High',
    'Father''s name listed as John Doe Sr.',
    4
  );

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
    'Mother''s Name: Mary Smith',
    'Citation',
    'County Clerk',
    'Birth Certificate of John Doe, Entry 456',
    'High',
    'Mother''s maiden name listed as Mary Smith',
    4
  );

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
    'Marriage Record of John Doe and Jane Smith',
    'Marriage Record',
    'County Clerk''s Office',
    'County Records, Jamestown, USA',
    'High',
    'Original document, certified copy.',
    NULL
  );

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
    'Marriage Date: July 25, 1895',
    'Citation',
    'County Clerk',
    'Marriage Record of John Doe and Jane Smith, Entry 123',
    'High',
    'Date of Marriage: July 25, 1895',
    8
  );

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
    '1900 US Census, Jamestown, Jamesstate',
    'Census Record',
    'US Census Bureau',
    'National Archives and Records Administration',
    'Medium',
    'Information provided by head of household. Potential for errors.',
    NULL
  );

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
    'John Doe Household, 1900 Census',
    'Citation',
    'US Census Bureau',
    '1900 US Census, Jamestown, Jamesstate, ED 12, Sheet 5, Line 23',
    'Medium',
    'Lists John Doe, age 28, occupation: Farmer',
    10
  );

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
    'Jane Doe (Wife), 1900 Census',
    'Citation',
    'US Census Bureau',
    '1900 US Census, Jamestown, Jamesstate, ED 12, Sheet 5, Line 24',
    'Medium',
    'Lists Jane Doe, age 26, wife',
    10
  );

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
    'Death Certificate of Mary Smith (nee Brown)',
    'Death Certificate',
    'State Department of Health',
    'State Vital Records, Jamesstate, USA',
    'High',
    'Original document, certified copy.  Indicates maiden name.',
    NULL
  );

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
    'Mary Smith Date of Death',
    'Citation',
    'State Department of Health',
    'Death Certificate of Mary Smith, Certificate Number 12345',
    'High',
    'Date of Death: March 10, 1910',
    13
  );

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
    'Mary Smith Place of Birth',
    'Citation',
    'State Department of Health',
    'Death Certificate of Mary Smith, Certificate Number 12345',
    'Medium',
    'Place of Birth: Jamestown, Jamesstate',
    13
  );

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
    'Abstract of Will for John Doe Sr.',
    'Will Abstract',
    'County Clerk''s Office',
    'Will Book 4, Page 123',
    'Medium',
    'Summarized record, contains key information, but potential omissions',
    NULL
  );

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
    'Mention of Wife, Mary',
    'Citation',
    'County Clerk',
    'Abstract of Will for John Doe Sr., Page 123, line 10',
    'Medium-High',
    'Will proved on June, 2nd 1875, mentions wife Mary Doe',
    16
  );

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
    'Mention of Son, John',
    'Citation',
    'County Clerk',
    'Abstract of Will for John Doe Sr., Page 124, line 1',
    'Medium-High',
    'Mentions son, John Doe',
    16
  );

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
    'Birth Certificate of Jane Smith',
    'Birth Certificate',
    'County Clerk''s Office',
    'County Records, Jamestown, USA',
    'High',
    'Original document, certified copy.',
    NULL
  );

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
    'Father: Robert Smith',
    'Citation',
    'County Clerk',
    'Birth Certificate of Jane Smith, Entry 789',
    'High',
    'Father''s name listed as Robert Smith',
    19
  );

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
    'Mother: Emily Green',
    'Citation',
    'County Clerk',
    'Birth Certificate of Jane Smith, Entry 789',
    'High',
    'Mother''s maiden name listed as Emily Green',
    19
  );
