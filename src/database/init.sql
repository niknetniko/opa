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

INSERT INTO event_roles (role, builtin)
VALUES ('Primary', true),
       ('Partner', true),
       ('Witness', true),
       ('Mother', true),
       ('Father', true),
       ('AdoptiveParent', true),
       ('Stepparent', true),
       ('FosterParent', true),
       ('SurrogateMother', true),
       ('GeneticDonor', true),
       ('RecognizedParent', true);

INSERT INTO event_types(type, builtin)
VALUES ('Birth', true),
       ('Death', true),
       ('Marriage', true),
       ('Divorce', true),
       ('Baptism', true),
       ('Funeral', true);
