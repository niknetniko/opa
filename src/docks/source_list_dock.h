
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
/**
 * Show a filterable list of people.
 *
 * When a person is selected, a new tab with details about that person will be shown.
 */
class SourceTreeWidget : public QWidget {
    Q_OBJECT

public:
    explicit SourceTreeWidget(QWidget* parent);

    ~SourceTreeWidget() override = default;

public Q_SLOTS:
    void handleSelectedNewRow(const QItemSelection& selected);

Q_SIGNALS:
    /**
     * Called when a new person is selected by the table view.
     */
    void handleSourceSelected(IntegerPrimaryKey sourceId);

private:
    QTreeView* treeView;
};

/**
 * Wrapper around a person list view, allowing it to act as a dock.
 */
class SourceListDock : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit SourceListDock();

Q_SIGNALS:
    /**
     * Called when a new person is selected by the table view.
     */
    void handleSourceSelected(IntegerPrimaryKey sourceId);
};
