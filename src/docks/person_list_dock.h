
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "database/schema.h"
#include <kddockwidgets/DockWidget.h>

#include <QTableView>

/**
 * Show a filterable list of people.
 *
 * When a person is selected, a new tab with details about that person will be shown.
 */
class PersonListWidget : public QWidget {
    Q_OBJECT

public:
    explicit PersonListWidget(QWidget* parent);

    ~PersonListWidget() override = default;

public Q_SLOTS:
    void handleSelectedNewRow(const QItemSelection& selected);

Q_SIGNALS:
    /**
     * Called when a new person is selected by the table view.
     */
    void handlePersonSelected(IntegerPrimaryKey personId);

private:
    QTableView* tableView;
};

/**
 * Wrapper around a person list view, allowing it to act as a dock.
 */
class PersonListDock : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit PersonListDock();

Q_SIGNALS:
    /**
     * Called when a new person is selected by the table view.
     */
    void handlePersonSelected(IntegerPrimaryKey personId);
};
