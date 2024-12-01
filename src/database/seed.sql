INSERT INTO people (id, root, sex)
VALUES (1, TRUE, 'Male');

INSERT INTO people (id, root, sex)
VALUES (2, FALSE, 'Female');

INSERT INTO people (id, root, sex)
VALUES (3, FALSE, 'Male');
INSERT INTO people (id, root, sex)
VALUES (4, FALSE, 'Female');

INSERT INTO people (id, root, sex)
VALUES (5, FALSE, 'Male');
INSERT INTO people (id, root, sex)
VALUES (6, FALSE, 'Female');
INSERT INTO people (id, root, sex)
VALUES (7, FALSE, 'Male');
INSERT INTO people (id, root, sex)
VALUES (8, FALSE, 'Female');

INSERT INTO names (id, person_id, sort, titles, given_names, prefix, surname, note, origin_id)
VALUES (1, 1, 1, '', 'John', '', 'Doe', '', 1),
       (2, 2, 1, '', 'Jane', '', 'Doe', '', 1),
       (3, 3, 1, '', 'Michael', '', 'Doe', '', 1),
       (4, 4, 1, '', 'Emily', '', 'Smith', '', 1),
       (5, 5, 1, '', 'William', '', 'Doe', '', 1),
       (6, 6, 1, '', 'Elizabeth', '', 'Brown', '', 1),
       (7, 7, 1, '', 'George', '', 'Smith', '', 1),
       (8, 8, 1, '', 'Mary', '', 'Taylor', '', 1);

INSERT INTO events (id, type_id, date, name, note)
VALUES (1, 1,
        '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
        'Birth of John', ''),
       (2, 1,
        '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
        'Birth of Jane', ''),
       (6, 1,
        '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
        'Birth of Michael', ''),
       (7, 1,
        '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
        'Birth of Emily', ''),
       (8, 1,
        '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
        'Birth of William', ''),
       (9, 1,
        '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
        'Birth of Elizabeth', ''),
       (10, 1,
        '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
        'Birth of George', ''),
       (11, 1,
        '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
        'Birth of Mary', '');

INSERT INTO event_relations (event_id, person_id, role_id)
VALUES (1, 1, 1),
       (2, 2, 1),
       (6, 3, 1),
       (7, 4, 1),
       (8, 5, 1),
       (9, 6, 1),
       (10, 7, 1),
       (11, 8, 1);

INSERT INTO event_relations (event_id, person_id, role_id)
VALUES (1, 3, 5),
       (1, 4, 4),
       (2, 3, 5),
       (2, 4, 4),
       (6, 5, 5),
       (6, 6, 4),
       (7, 7, 5),
       (7, 8, 4);

INSERT INTO events (id, type_id, date, name, note)
VALUES (3, 3,
        '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
        'Marriage of Michael and Emily', ''),
       (4, 3,
        '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
        'Marriage of William and Elizabeth', ''),
       (5, 3,
        '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
        'Marriage of George and Mary', '');

INSERT INTO event_relations (event_id, person_id, role_id)
VALUES (3, 3, 1),
       (3, 4, 2),
       (4, 5, 1),
       (4, 6, 2),
       (5, 7, 1),
       (5, 8, 2);

INSERT INTO events (id, type_id, date, name, note)
VALUES (12, 2,
        '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
        'Death of William', ''),
       (13, 2,
        '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
        'Death of Elizabeth', ''),
       (14, 2,
        '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
        'Death of George', ''),
       (15, 2,
        '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
        'Death of Mary', '');

INSERT INTO event_relations (event_id, person_id, role_id)
VALUES (12, 5, 1),
       (13, 6, 1),
       (14, 7, 1),
       (15, 8, 1);
