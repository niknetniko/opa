/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "person_family_tab.h"

#include "data/data_manager.h"
#include "data/family.h"
#include "utils/model_utils_find_source_model_of_type.h"

#include <KLocalizedString>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>


PersonFamilyTab::PersonFamilyTab(IntegerPrimaryKey person, QWidget* parent) : QWidget(parent) {
    personId = person;

    auto* familyModel = DataManager::get().familyModelFor(this, personId);

    // Create the tree view.
    treeView = new QTreeView(this);
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    treeView->setModel(familyModel);
    treeView->setUniformRowHeights(true);
    treeView->expandAll();

    auto* toolbar = new QToolBar(this);

    auto* addPartnerAction = new QAction(toolbar);
    addPartnerAction->setText(i18n("Add new partner"));
    addPartnerAction->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    toolbar->addAction(addPartnerAction);

    auto* nameTabContainerLayout = new QVBoxLayout(this);
    nameTabContainerLayout->addWidget(toolbar);
    nameTabContainerLayout->addWidget(treeView);
}
