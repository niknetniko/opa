INSERT INTO people(root, sex)
VALUES (true, 'male'),
       (false, 'female');

INSERT INTO name_origins(origin, builtin)
VALUES ('Unknown', true),
       ('Inherited', true),
       ('Patrilineal', true),
       ('Matrilineal', true),
       ('Taken', true),
       ('Patronymic', true),
       ('Matronymic', true),
       ('Location', true),
       ('Occupation', true);

INSERT INTO names(person_id, sort, given_names, surname, origin_id)
VALUES (1, 1, 'Jos', 'Jozephson', 1);

INSERT INTO names(person_id, sort, given_names, surname, origin_id)
VALUES (2, 1, 'Jan', 'Jaap', 2),
       (2, 2, 'Aap', 'Daap', 1);

INSERT INTO event_roles (role, builtin)
VALUES ('Primary', true),
       ('Witness', true),
       ('Mother', true),
       ('Father', true),
       ('Adoptive parent', true),
       ('Stepparent', true),
       ('Foster parent', true),
       ('Surrogate mother', true),
       ('Genetic donor', true),
       ('Recognized parent', true);

INSERT INTO event_types(type, builtin)
VALUES ('Birth', true),
       ('Death', true),
       ('Marriage', true),
       ('Divorce', true),
       ('Baptism', true),
       ('Funeral', true);

INSERT INTO events (type_id, date, name)
VALUES (1, NULL, 'test 1'),
       (2, NULL, 'test 2');

INSERT INTO event_relations (event_id, person_id, role_id)
VALUES (1, 1, 1),
       (2, 2, 2);
