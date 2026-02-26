/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "event_entities.h"
#include "model/object_table_model.h"

/**
 * Provides only event roles that are parent roles (Father, Mother, etc.),
 * populated directly from the EventRepository.
 */
class ParentEventRolesListModel : public ObjectTableModel<EventRoleEntity> {
    Q_OBJECT

public:
    enum Columns { ID = 0, ROLE, BUILTIN };
    Q_ENUM(Columns)

    explicit ParentEventRolesListModel(QObject* parent = nullptr);

public Q_SLOTS:
    void reload();
};

/**
 * Provides only event types that represent the start of a relationship
 * (Marriage, etc.), populated directly from the EventRepository.
 */
class RelationshipEventTypesListModel : public ObjectTableModel<EventTypeEntity> {
    Q_OBJECT

public:
    enum Columns { ID = 0, TYPE, BUILTIN };
    Q_ENUM(Columns)

    explicit RelationshipEventTypesListModel(QObject* parent = nullptr);

public Q_SLOTS:
    void reload();
};
