/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "person_list_dock.h"

#include "data/data_manager.h"
#include "data/person.h"
#include "utils/formatted_identifier_delegate.h"

#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>

PersonListDock::PersonListDock() : DockWidget(QStringLiteral("People"), KDDockWidgets::DockWidgetOption_DeleteOnClose) {
    auto* tableView = new PersonListWidget(this);
    setWidget(tableView);

    connect(tableView, &PersonListWidget::handlePersonSelected, this, &PersonListDock::handlePersonSelected);
}

PersonListWidget::PersonListWidget(QWidget* parent) : QWidget(parent) {
    auto* baseModel = DataManager::get().primaryNamesModel(this);

    // Create a searchable model.
    auto* filtered = new QSortFilterProxyModel(this);
    filtered->setSourceModel(baseModel);
    filtered->setFilterKeyColumn(DisplayNameModel::NAME);
    filtered->setFilterCaseSensitivity(Qt::CaseInsensitive);

    auto* searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText(i18n("Search.."));
    searchBox->setClearButtonEnabled(true);

    // Allow searching...
    connect(searchBox, &QLineEdit::textEdited, filtered, &QSortFilterProxyModel::setFilterFixedString);

    tableView = new QTableView(this);
    tableView->setModel(filtered);
    tableView->setShowGrid(false);
    tableView->setSelectionBehavior(QTableView::SelectRows);
    tableView->setSelectionMode(QTableView::SelectionMode::SingleSelection);
    tableView->setSortingEnabled(true);
    tableView->verticalHeader()->hide();
    tableView->horizontalHeader()->resizeSections(QHeaderView::Stretch);
    tableView->horizontalHeader()->setSectionResizeMode(DisplayNameModel::ID, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(DisplayNameModel::NAME, QHeaderView::Stretch);
    tableView->horizontalHeader()->setSectionResizeMode(DisplayNameModel::ROOT, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setHighlightSections(false);
    tableView->setItemDelegateForColumn(
        DisplayNameModel::ID, new FormattedIdentifierDelegate(tableView, FormattedIdentifierDelegate::PERSON)
    );

    // Wrap in a VBOX for layout reasons.
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(searchBox);
    layout->addWidget(tableView);

    connect(
        tableView->selectionModel(),
        &QItemSelectionModel::selectionChanged,
        this,
        &PersonListWidget::handleSelectedNewRow
    );
}

void PersonListWidget::handleSelectedNewRow(const QItemSelection& selected) {
    if (selected.empty()) {
        return;
    }

    // Get the ID of the person we want.
    auto personId = getIdFromSelection(selected, tableView->model(), DisplayNameModel::ID);
    Q_EMIT handlePersonSelected(personId);
}
