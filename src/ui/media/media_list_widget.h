/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"
#include "domain/media/media_entities.h"
#include "model/object_table_model.h"

#include <QWidget>
#include <functional>

class QTableView;
class QAction;

/**
 * Reusable widget that displays a list of media attachments for any entity.
 *
 * Callers supply three lambdas for loading, attaching and detaching media, making
 * this widget independent of the specific entity type it is embedded in.
 */
class MediaListWidget : public QWidget {
    Q_OBJECT

public:
    using LoadFn   = std::function<QList<MediaEntity>()>;
    using AttachFn = std::function<bool(IntegerPrimaryKey mediaId)>;
    using DetachFn = std::function<bool(IntegerPrimaryKey mediaId)>;

    explicit MediaListWidget(LoadFn load, AttachFn attach, DetachFn detach, QWidget* parent = nullptr);

    void reload();

private:
    LoadFn loadFn;
    AttachFn attachFn;
    DetachFn detachFn;

    QTableView* tableView;
    ObjectTableModel<MediaEntity>* model;
    QAction* addAction;
    QAction* linkAction;
    QAction* editAction;
    QAction* openAction;
    QAction* removeAction;

    void onAddFile();
    void onLinkExisting();
    void onEdit();
    void onOpen() const;
    void onRemove();
    void updateButtons() const;
};
