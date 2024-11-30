INSERT INTO people(root, sex)
VALUES (true, 'male'),
       (false, 'female'),
       (false, 'male'),
       (false, 'male'),
       (false, 'female');

INSERT INTO names(person_id, sort, given_names, surname, origin_id)
VALUES (1, 1, 'Jos', 'Jozephson', 1);

INSERT INTO names(person_id, sort, given_names, surname, origin_id)
VALUES (2, 1, 'Jan', 'Jaap', 2),
       (2, 2, 'Aap', 'Daap', 1);

INSERT INTO names(person_id, sort, given_names, surname, origin_id)
VALUES (3, 1, 'Zoon', 'Jozephson', 1),
       (4, 1, 'Random person', 'Jozephson', 1),
       (5, 1, 'Dochter', 'Jozephson', 1);

INSERT INTO events (type_id, date, name)
VALUES (1,
        '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
        'geboorte Jos'),
       (2, NULL, 'dood Jos'),
       (3, NULL, 'Marriage Jos'),
       (1,
        '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
        'geboorte zoon'),
       (1,
        '{"dateModifier":"NONE","dateQuality":"EXACT","day":true,"month":true,"proleptic":2451160,"userText":"","year":true}',
        'geboorte dochter');

INSERT INTO event_relations (event_id, person_id, role_id)
VALUES (1, 1, 1),
       (2, 2, 1),
       (3, 1, 1),
       (3, 2, 2),
       (4, 3, 1),
       (4, 1, 5),
       (4, 2, 4),
       (5, 1, 5),
       (5, 5, 1);


