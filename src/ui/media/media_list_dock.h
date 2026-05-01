/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <kddockwidgets/qtwidgets/views/DockWidget.h>

#include <QTableView>
#include <QWidget>

class MediaTableWidget : public QWidget {
    Q_OBJECT

public:
    explicit MediaTableWidget(QWidget* parent = nullptr);

private Q_SLOTS:
    void onContextMenuRequested(const QPoint& pos);
    void onDoubleClicked(const QModelIndex& index);

private:
    QTableView* tableView;
    void openEditDialog(IntegerPrimaryKey mediaId);
};

class MediaListDock : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit MediaListDock();
};
