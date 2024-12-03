/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "database/schema.h"

#include <QWidget>

class QTreeView;

/**
 * Wrapper around the FamilyTreeView to add toolbars and stuff.
 */
class PersonFamilyTab : public QWidget {
    Q_OBJECT

public:
    explicit PersonFamilyTab(IntegerPrimaryKey person, QWidget* parent);

public Q_SLOTS:
    // TODO: allow selecting people.

    void showPedigreeChart();

private:
    IntegerPrimaryKey personId;
    QTreeView* partnerAndDescendantTreeView;
    QTreeView* parentsTreeView;
};
