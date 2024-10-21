INSERT INTO people(root, sex)
VALUES (true, 'male');

-- These are based on those from Gramps.
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
VALUES (1, 1, 'Jos', 'Jozephson', 0);

INSERT INTO people(root, sex)
VALUES (true, 'male');

INSERT INTO names(person_id, sort, given_names, surname, origin_id)
VALUES (2, 1, 'Jan', 'Jaap', 1),
       (2, 2, 'Aap', 'Daap', 0);

-- Insert some "known" event roles
-- TODO: how do we want to handle these?
-- TODO: once we have parents, should that be a role? Probably not I guess?
INSERT INTO event_roles (role)
VALUES ('primary'),
       ('witness');

INSERT INTO event_types(type)
VALUES ('birth'),
       ('death'),
       ('baptism'),
       ('burial');

INSERT INTO events (type_id, date, name)
VALUES (1, NULL, 'test 1'),
       (2, NULL, 'test 2');

INSERT INTO event_relations (event_id, person_id, role_id)
VALUES (1, 1, 1),
       (2, 2, 2);

