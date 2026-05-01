/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "media_entities.h"
#include "model/object_table_model.h"

class MediaListModel : public ObjectTableModel<MediaEntity> {
    Q_OBJECT

public:
    enum Columns { ID = 0, TITLE, PATH, MIME_TYPE };
    Q_ENUM(Columns)

    explicit MediaListModel(QObject* parent = nullptr);

public Q_SLOTS:
    void reload();
};
