/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QMainWindow>

class QTreeView;
class QAction;
class QItemSelection;

class LocationManagementWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit LocationManagementWindow(QWidget* parent = nullptr);

private Q_SLOTS:
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void addRootLocation();
    void addChildLocation();
    void editSelectedLocation();
    void deleteSelectedLocation();

private:
    QTreeView* treeView;
    QAction* addChildAction;
    QAction* editAction;
    QAction* deleteAction;

    [[nodiscard]] IntegerPrimaryKey selectedLocationId() const;
};
