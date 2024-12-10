/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QTableView>
#include <QWidget>

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
    void onAddNewPerson();

Q_SIGNALS:
    /**
     * Called when a new person is selected by the table view.
     */
    void handlePersonSelected(IntegerPrimaryKey personId);

private:
    QTableView* tableView;
};
