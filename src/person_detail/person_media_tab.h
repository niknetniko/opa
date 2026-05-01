/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QWidget>

/**
 * Tab for displaying and managing media attachments for a given person.
 */
class PersonMediaTab : public QWidget {
    Q_OBJECT

public:
    explicit PersonMediaTab(IntegerPrimaryKey personId, QWidget* parent);
};
