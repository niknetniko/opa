
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "database/schema.h"

#include <QMainWindow>
#include <QWidget>

class QAbstractItemModel;

class TreeViewWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit TreeViewWindow(IntegerPrimaryKey person, QWidget* parent = nullptr);
};
