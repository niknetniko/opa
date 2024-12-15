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
    void onShowPedigreeChart() const;
    void onParentClicked(const QModelIndex& index) const;
    void onPartnerOrChildClicked(const QModelIndex& index) const;
    void onAddParent();

private:
    IntegerPrimaryKey personId;
    QTreeView* partnerAndDescendantTreeView;
    QTreeView* parentsTreeView;
};
