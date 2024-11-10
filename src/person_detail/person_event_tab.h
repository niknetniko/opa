/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QTableView>
#include <QWidget>

class PersonEventTab : public QWidget {
    Q_OBJECT

public:
    explicit PersonEventTab(IntegerPrimaryKey person, QWidget* parent);

public Q_SLOTS:
    void onEventSelected(const QAbstractItemModel* model, const QItemSelection& selected) const;

private:
    QAction* addAction;
    QAction* removeAction;
    QAction* unlinkAction;
    QAction* editAction;
};
