/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "person_family_tab.h"

#include "data/data_manager.h"
#include "data/family.h"
#include "utils/model_utils_find_source_model_of_type.h"
#include <tree_view/tree_view_window.h>

#include <KLocalizedString>
#include <QGroupBox>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>


PersonFamilyTab::PersonFamilyTab(IntegerPrimaryKey person, QWidget* parent) : QWidget(parent) {
    personId = person;

    auto* familyModel = DataManager::get().familyModelFor(this, personId);
    auto* sourceModel = findSourceModelOfType<FamilyProxyModel>(familyModel);

    auto* familyGroupBox = new QGroupBox(i18n("Partners and children"), this);
    familyGroupBox->setFlat(true);

    // Create the tree view.
    partnerAndDescendantTreeView = new QTreeView(familyGroupBox);
    partnerAndDescendantTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    partnerAndDescendantTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    partnerAndDescendantTreeView->setModel(familyModel);
    partnerAndDescendantTreeView->setUniformRowHeights(true);
    partnerAndDescendantTreeView->expandAll();
    if (sourceModel->hasBastardChildren()) {
        partnerAndDescendantTreeView->setFirstColumnSpanned(sourceModel->rowCount() - 1, {}, true);
    }

    auto* toolbar = new QToolBar(familyGroupBox);

    auto* addPartnerAction = new QAction(toolbar);
    addPartnerAction->setText(i18n("Add new partner"));
    addPartnerAction->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    toolbar->addAction(addPartnerAction);

    auto* familyLayout = new QVBoxLayout(familyGroupBox);
    familyLayout->addWidget(toolbar);
    familyLayout->addWidget(partnerAndDescendantTreeView);

    auto* parentsGroupBox = new QGroupBox(i18n("Parents"), this);
    parentsGroupBox->setFlat(true);

    auto* parentsModel = DataManager::get().parentsModelFor(this, person);
    parentsTreeView = new QTreeView(parentsGroupBox);
    parentsTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    parentsTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    parentsTreeView->setUniformRowHeights(true);
    parentsTreeView->setModel(parentsModel);
    parentsTreeView->setRootIsDecorated(false);
    parentsTreeView->setSortingEnabled(false);

    auto* parentsToolbar = new QToolBar(parentsGroupBox);

    auto* showPedigreeChart = new QAction(parentsToolbar);
    showPedigreeChart->setText(i18n("Show pedigree chart"));
    showPedigreeChart->setIcon(QIcon::fromTheme(QStringLiteral("distribute-graph-directed")));
    parentsToolbar->addAction(showPedigreeChart);
    connect(showPedigreeChart, &QAction::triggered, this, &PersonFamilyTab::showPedigreeChart);

    auto* parentsLayout = new QVBoxLayout(parentsGroupBox);
    parentsLayout->addWidget(parentsToolbar);
    parentsLayout->addWidget(parentsTreeView);

    // Main layout
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(parentsGroupBox);
    mainLayout->addWidget(familyGroupBox);
    mainLayout->setStretch(0, 0);
    mainLayout->setStretch(1, 1);
}

void PersonFamilyTab::showPedigreeChart() {
    auto* pedigree = new TreeViewWindow(personId, this);
    pedigree->show();
}
