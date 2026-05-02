/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "location_management_window.h"

#include "domain/location/location_list_model.h"
#include "domain/location/location_repository.h"
#include "editors/location_editor_dialog.h"
#include "utils/formatted_identifier_delegate.h"
#include "utils/tree_proxy_model.h"

#include <KLocalizedString>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

using namespace Qt::StringLiterals;

LocationManagementWindow::LocationManagementWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(i18n("Manage locations"));

    auto* locationsModel = new LocationListModel(this);

    auto* treeModel = new TreeProxyModel(this);
    treeModel->setSourceModel(locationsModel);
    treeModel->setIdColumn(LocationListModel::ID);
    treeModel->setParentIdColumn(LocationListModel::PARENT_ID);

    auto* filtered = new QSortFilterProxyModel(this);
    filtered->setSourceModel(treeModel);
    filtered->setFilterKeyColumn(LocationListModel::NAME);
    filtered->setFilterCaseSensitivity(Qt::CaseInsensitive);
    filtered->setRecursiveFilteringEnabled(true);

    auto* searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText(i18n("Search..."));
    searchBox->setClearButtonEnabled(true);
    connect(searchBox, &QLineEdit::textEdited, filtered, &QSortFilterProxyModel::setFilterFixedString);

    treeView = new QTreeView(this);
    treeView->setModel(filtered);
    treeView->setSelectionBehavior(QTreeView::SelectRows);
    treeView->setSelectionMode(QTreeView::SingleSelection);
    treeView->setSortingEnabled(true);
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    treeView->header()->setSectionResizeMode(LocationListModel::NAME, QHeaderView::Stretch);
    treeView->setItemDelegateForColumn(
        LocationListModel::ID,
        new FormattedIdentifierDelegate(treeView, FormattedIdentifierDelegate::LOCATION)
    );
    treeView->hideColumn(LocationListModel::PARENT_ID);
    treeView->hideColumn(LocationListModel::TYPE_ID);
    treeView->setUniformRowHeights(true);
    treeView->expandToDepth(1);

    connect(
        treeView->selectionModel(),
        &QItemSelectionModel::selectionChanged,
        this,
        &LocationManagementWindow::onSelectionChanged
    );

    auto* toolBar = addToolBar(i18n("Actions"));

    auto* addRootAction = new QAction(QIcon::fromTheme(u"list-add"_s), i18n("Add root location"), this);
    connect(addRootAction, &QAction::triggered, this, &LocationManagementWindow::addRootLocation);
    toolBar->addAction(addRootAction);

    addChildAction = new QAction(QIcon::fromTheme(u"list-add"_s), i18n("Add child location"), this);
    addChildAction->setEnabled(false);
    connect(addChildAction, &QAction::triggered, this, &LocationManagementWindow::addChildLocation);
    toolBar->addAction(addChildAction);

    editAction = new QAction(QIcon::fromTheme(u"document-edit"_s), i18n("Edit location"), this);
    editAction->setEnabled(false);
    connect(editAction, &QAction::triggered, this, &LocationManagementWindow::editSelectedLocation);
    toolBar->addAction(editAction);

    deleteAction = new QAction(QIcon::fromTheme(u"edit-delete"_s), i18n("Delete location"), this);
    deleteAction->setEnabled(false);
    connect(deleteAction, &QAction::triggered, this, &LocationManagementWindow::deleteSelectedLocation);
    toolBar->addAction(deleteAction);

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    layout->addWidget(searchBox);
    layout->addWidget(treeView);
    setCentralWidget(central);
}

void LocationManagementWindow::onSelectionChanged(
    const QItemSelection& selected,
    [[maybe_unused]] const QItemSelection& deselected
) {
    const bool hasSelection = !selected.isEmpty();
    addChildAction->setEnabled(hasSelection);
    editAction->setEnabled(hasSelection);
    deleteAction->setEnabled(hasSelection);
}

IntegerPrimaryKey LocationManagementWindow::selectedLocationId() const {
    const auto indexes = treeView->selectionModel()->selectedRows(LocationListModel::ID);
    if (indexes.isEmpty()) {
        return -1;
    }
    return indexes.first().data(Qt::EditRole).value<IntegerPrimaryKey>();
}

void LocationManagementWindow::addRootLocation() {
    LocationEditorDialog::showDialogForNewLocation(std::nullopt, this);
}

void LocationManagementWindow::addChildLocation() {
    const auto parentId = selectedLocationId();
    if (parentId < 0) {
        return;
    }
    LocationEditorDialog::showDialogForNewLocation(parentId, this);
}

void LocationManagementWindow::editSelectedLocation() {
    const auto id = selectedLocationId();
    if (id < 0) {
        return;
    }
    LocationEditorDialog::showDialogForExistingLocation(id, this);
}

void LocationManagementWindow::deleteSelectedLocation() {
    const auto id = selectedLocationId();
    if (id < 0) {
        return;
    }

    LocationRepository repo;
    if (repo.isUsed(id)) {
        QMessageBox::warning(
            this,
            i18n("Cannot delete location"),
            i18n("This location is in use by one or more events and cannot be deleted.")
        );
        return;
    }

    if (repo.hasChildren(id)) {
        const auto childResult = QMessageBox::warning(
            this,
            i18n("Delete location with children"),
            i18n("This location has child locations. Deleting it will make those locations parentless. Continue?"),
            QMessageBox::Yes | QMessageBox::No
        );
        if (childResult != QMessageBox::Yes) {
            return;
        }
    }

    const auto result = QMessageBox::warning(
        this,
        i18n("Delete location"),
        i18n("Are you sure you want to delete this location?"),
        QMessageBox::Yes | QMessageBox::No
    );
    if (result == QMessageBox::Yes) {
        if (!repo.deleteLocation(id)) {
            QMessageBox::critical(
                this,
                i18n("Delete failed"),
                i18n("Could not delete the location. Please try again.")
            );
        }
    }
}
