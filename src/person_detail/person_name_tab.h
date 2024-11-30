/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QWidget>


class QAbstractItemModel;
class QItemSelection;
class QTreeView;
class QAction;

/**
 * Tab for displaying and managing the names of a given person.
 */
class PersonNameTab : public QWidget {
    Q_OBJECT

public:
    explicit PersonNameTab(IntegerPrimaryKey person, QWidget* parent);

public Q_SLOTS:
    /**
     * Called when a name is selected in the table.
     *
     * @param selected The selection (can only be one at a time).
     */
    void onNameSelected(const QItemSelection& selected) const;

    /**
     * Called when the sort changes.
     */
    void onSortChanged(const QItemSelection& selected, int logicalIndex) const;

    /**
     * Initiate adding a new name.
     */
    void onAddNewName();

    /**
     * Initiate editing the currently selected name.
     */
    void onEditSelectedName();

    /**
     * Remove the currently selected name.
     */
    void onRemoveSelectedName() const;

    /**
     * Move the selected name down one row.
     */
    void onMoveSelectedNameDown() const;

    /**
     * Move the selected name up one row.
     */
    void onMoveSelectedNameUp() const;

private:
    IntegerPrimaryKey person;

    QAction* addAction;
    QAction* removeAction;
    QAction* editAction;
    QAction* upAction;
    QAction* downAction;

    QTreeView* treeView;
    QAbstractItemModel* baseModel;

    /**
     * Move a name from one row to another row.
     *
     * @param sourceRow The original, current row in the tree view.
     * @param destinationRow The destination row in the tree view.
     */
    void moveSelectedNameToPosition(int sourceRow, int destinationRow) const;
};
