/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QWidget>

class QAction;
class QTreeView;
class QItemSelection;
class QAbstractItemModel;

class PersonEventTab : public QWidget {
    Q_OBJECT

public:
    explicit PersonEventTab(IntegerPrimaryKey person, QWidget* parent);

public Q_SLOTS:
    /**
     * Called when an event is selected in the table.
     */
    void onEventSelected(const QItemSelection& selected) const;

    /**
     * Initiate adding a new name.
     */
    void onAddNewEvent();

    /**
     * Initiate editing the currently selected name.
     */
    void onEditSelectedEvent();

    /**
     * Remove the currently selected name.
     */
    void onRemoveSelectedEvent() const;

    /**
     * Unlink the selected event.
     */
    void onUnlinkSelectedEvent();

    /**
     * Link an existing event.
     */
    void onLinkExistingEvent();

private:
    IntegerPrimaryKey person;

    QAction* addAction;
    QAction* removeAction;
    QAction* linkAction;
    QAction* unlinkAction;
    QAction* editAction;

    QTreeView* treeView;
    QAbstractItemModel* baseModel;
};
