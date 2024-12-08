/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "person_family_tab.h"

#include "data/data_manager.h"
#include "data/family.h"
#include "main/main_window.h"
#include "tree_view/tree_view_window.h"
#include "utils/formatted_identifier_delegate.h"
#include "utils/model_utils_find_source_model_of_type.h"

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
    partnerAndDescendantTreeView->setItemDelegateForColumn(
        FamilyDisplayModel::PERSON_ID, new FormattedIdentifierDelegate(this, FormattedIdentifierDelegate::PERSON)
    );
    partnerAndDescendantTreeView->setItemDelegateForColumn(
        FamilyDisplayModel::EVENT_ID, new FormattedIdentifierDelegate(this, FormattedIdentifierDelegate::EVENT)
    );
    if (sourceModel->hasBastardChildren()) {
        partnerAndDescendantTreeView->setFirstColumnSpanned(sourceModel->rowCount() - 1, {}, true);
    }
    connect(partnerAndDescendantTreeView, &QTreeView::doubleClicked, this, &PersonFamilyTab::onPartnerOrChildClicked);

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
    parentsTreeView->setMinimumHeight(100);
    parentsTreeView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    parentsTreeView->setItemDelegateForColumn(
        DisplayParentModel::PERSON_ID, new FormattedIdentifierDelegate(this, FormattedIdentifierDelegate::PERSON)
    );
    connect(parentsTreeView, &QTreeView::doubleClicked, this, &PersonFamilyTab::onParentClicked);

    auto* parentsToolbar = new QToolBar(parentsGroupBox);

    auto* showPedigreeChart = new QAction(parentsToolbar);
    showPedigreeChart->setText(i18n("Show pedigree chart"));
    showPedigreeChart->setIcon(QIcon::fromTheme(QStringLiteral("distribute-graph-directed")));
    parentsToolbar->addAction(showPedigreeChart);
    connect(showPedigreeChart, &QAction::triggered, this, &PersonFamilyTab::onShowPedigreeChart);

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

void PersonFamilyTab::onShowPedigreeChart() const {
    auto* pedigree = new TreeViewWindow(personId);
    pedigree->show();
}

void PersonFamilyTab::onParentClicked(const QModelIndex& index) const {
    if (!index.isValid()) {
        return;
    }
    assert(index.model() == parentsTreeView->model());
    auto personId = index.model()->index(index.row(), DisplayParentModel::PERSON_ID, index.parent()).data();
    openOrSelectPerson(personId.toLongLong());
}

void PersonFamilyTab::onPartnerOrChildClicked(const QModelIndex& index) const {
    if (!index.isValid()) {
        return;
    }
    assert(index.model() == partnerAndDescendantTreeView->model());
    auto personId = index.model()->index(index.row(), FamilyDisplayModel::PERSON_ID, index.parent()).data();
    openOrSelectPerson(personId.toLongLong());
}
