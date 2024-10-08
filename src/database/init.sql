INSERT INTO people(root, sex)
VALUES (true, 'male');

INSERT INTO name_origins(id, origin)
VALUES (0, 'unknown');
INSERT INTO name_origins(id, origin)
VALUES (1, 'patrilineal');

INSERT INTO names(person_id, sort, given_names, surname, origin_id)
VALUES (1, 1, 'Jos', 'Jozephson', 0);

INSERT INTO people(root, sex)
VALUES (true, 'male');

INSERT INTO names(person_id, sort, given_names, surname, origin_id)
VALUES (2, 1, 'Jan', 'Jaap', 1), (2, 2, 'Aap', 'Daap', 0);