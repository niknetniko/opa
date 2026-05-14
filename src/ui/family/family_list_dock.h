/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

#include <QTreeView>
#include <QWidget>

class QItemSelection;

class FamilyListWidget : public QWidget {
    Q_OBJECT

public:
    explicit FamilyListWidget(QWidget* parent);

    ~FamilyListWidget() override = default;

public Q_SLOTS:
    void handleSelectedNewRow(const QItemSelection& selected);

Q_SIGNALS:
    void familySelected(IntegerPrimaryKey familyId);
    void personSelected(IntegerPrimaryKey personId);

private:
    QTreeView* treeView;
};

class FamilyListDock : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit FamilyListDock();

Q_SIGNALS:
    void familySelected(IntegerPrimaryKey familyId);
    void personSelected(IntegerPrimaryKey personId);
};
