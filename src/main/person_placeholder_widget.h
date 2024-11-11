/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QWidget>

class PersonPlaceholderWidget : public QWidget {
    Q_OBJECT

public:
    explicit PersonPlaceholderWidget(QWidget* parent = nullptr);
};
