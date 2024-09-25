INSERT INTO people(root, sex)
VALUES (true, 'male');

INSERT INTO name_origins(id, origin)
VALUES (0, 'unknown');
INSERT INTO name_origins(id, origin)
VALUES (1, 'patrilinear');

INSERT INTO names(person_id, given_names, surname, main, origin_id)
VALUES (1, 'Niko', 'Strijbol', true, 0);

INSERT INTO people(root, sex)
VALUES (true, 'male');

INSERT INTO names(person_id, given_names, surname, main, origin_id)
VALUES (2, 'Jan', 'Jaap', true, 1), (2, 'Aap', 'Daap', false, 0);