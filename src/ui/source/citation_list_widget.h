/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"
#include "domain/source/source_entities.h"
#include "model/object_table_model.h"

#include <QWidget>
#include <functional>

class QTableView;
class QPushButton;

class CitationListWidget : public QWidget {
    Q_OBJECT

public:
    using LoadFn = std::function<QList<SourceEntity>()>;
    using AddFn = std::function<bool(IntegerPrimaryKey sourceId)>;
    using RemoveFn = std::function<bool(IntegerPrimaryKey sourceId)>;

    explicit CitationListWidget(LoadFn load, AddFn add, RemoveFn remove, QWidget* parent = nullptr);

    void reload();

private:
    LoadFn loadFn;
    AddFn addFn;
    RemoveFn removeFn;

    QTableView* tableView;
    ObjectTableModel<SourceEntity>* model;
    QPushButton* addNewButton;
    QPushButton* linkExistingButton;
    QPushButton* removeButton;

    void onAddNew();
    void onLinkExisting();
    void onRemove();
    void updateRemoveButton() const;
};
