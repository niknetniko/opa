/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "family_list_dock.h"

#include "domain/family/family_list_model.h"
#include "utils/formatted_identifier_delegate.h"

#include <KLocalizedString>
#include <QHeaderView>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>

FamilyListWidget::FamilyListWidget(QWidget* parent) : QWidget(parent) {
    auto* familyModel = new FamilyListModel(this);

    auto* filtered = new QSortFilterProxyModel(this);
    filtered->setSourceModel(familyModel);
    filtered->setFilterKeyColumn(FamilyListModel::DISPLAY_NAME);
    filtered->setFilterCaseSensitivity(Qt::CaseInsensitive);
    filtered->setRecursiveFilteringEnabled(true);

    auto* searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText(i18n("Search.."));
    searchBox->setClearButtonEnabled(true);
    connect(searchBox, &QLineEdit::textEdited, filtered, &QSortFilterProxyModel::setFilterFixedString);

    treeView = new QTreeView(this);
    treeView->setModel(filtered);
    treeView->setSelectionBehavior(QTreeView::SelectRows);
    treeView->setSelectionMode(QTreeView::SingleSelection);
    treeView->setSortingEnabled(true);
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    treeView->header()->setSectionResizeMode(FamilyListModel::DISPLAY_NAME, QHeaderView::Stretch);
    treeView->setItemDelegateForColumn(
        FamilyListModel::PERSON_ID,
        new FormattedIdentifierDelegate(treeView, FormattedIdentifierDelegate::PERSON)
    );
    treeView->setItemDelegateForColumn(
        FamilyListModel::EVENT_ID,
        new FormattedIdentifierDelegate(treeView, FormattedIdentifierDelegate::EVENT)
    );
    treeView->hideColumn(FamilyListModel::FAMILY_ID);
    treeView->setUniformRowHeights(true);
    treeView->expandToDepth(1);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(searchBox);
    layout->addWidget(treeView);

    connect(
        treeView->selectionModel(),
        &QItemSelectionModel::selectionChanged,
        this,
        &FamilyListWidget::handleSelectedNewRow
    );
}

void FamilyListWidget::handleSelectedNewRow(const QItemSelection& selected) {
    if (selected.empty()) {
        return;
    }
    auto index = selected.indexes().first();
    if (index.parent().isValid()) {
        auto personId =
            treeView->model()->index(index.row(), FamilyListModel::PERSON_ID, index.parent()).data().toLongLong();
        if (personId > 0) {
            Q_EMIT personSelected(personId);
        }
        return;
    }
    auto familyId =
        treeView->model()->index(index.row(), FamilyListModel::FAMILY_ID, index.parent()).data().toLongLong();
    if (familyId > 0) {
        Q_EMIT familySelected(familyId);
    }
}

FamilyListDock::FamilyListDock() :
    DockWidget(QStringLiteral("Families"), KDDockWidgets::DockWidgetOption_DeleteOnClose) {
    auto* widget = new FamilyListWidget(this);
    setWidget(widget);
    connect(widget, &FamilyListWidget::familySelected, this, &FamilyListDock::familySelected);
    connect(widget, &FamilyListWidget::personSelected, this, &FamilyListDock::personSelected);
}
