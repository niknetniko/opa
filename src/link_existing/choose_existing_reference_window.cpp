
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "choose_existing_reference_window.h"

#include <person_detail/person_event_tab.h>

#include <KLocalizedString>
#include <QHeaderView>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVBoxLayout>

QVariant ChooseExistingReferenceWindow::selectItem(
    int resultColumn, QAbstractItemModel* sourceModel, QWidget* parent, int searchColumn
) {
    assert(
        sourceModel->checkIndex(sourceModel->index(0, resultColumn), QAbstractItemModel::CheckIndexOption::IndexIsValid)
    );
    assert(
        searchColumn == -1 ||
        sourceModel->checkIndex(sourceModel->index(0, searchColumn), QAbstractItemModel::CheckIndexOption::IndexIsValid)
    );

    ChooseExistingReferenceWindow dialog(searchColumn, resultColumn, sourceModel, parent);
    dialog.exec();
    return dialog.selected;
}

void ChooseExistingReferenceWindow::accept() {
    if (auto selection = tableView->selectionModel()->selection(); !selection.isEmpty()) {
        const int selectedRow = selection.indexes().first().row();
        selected = tableView->model()->index(selectedRow, resultColumn).data();
    }

    QDialog::accept();
}

void ChooseExistingReferenceWindow::itemSelected(const QModelIndex& selected) {
    if (!selected.isValid()) {
        return;
    }

    accept();
}

ChooseExistingReferenceWindow::ChooseExistingReferenceWindow(
    int searchColumn, int resultColumn, QAbstractItemModel* sourceModel, QWidget* parent
) :
    QDialog(parent),
    searchColumn(searchColumn),
    resultColumn(resultColumn),
    sourceModel(sourceModel) {
    // TODO: allow setting the displayed columns somehow.
    // TODO: should I subclass this?
    // TODO: format IDs (in subclass somehow?)

    setWindowModality(Qt::WindowModal);

    auto* filterModel = new QSortFilterProxyModel(this);
    filterModel->setSourceModel(sourceModel);
    filterModel->setFilterKeyColumn(searchColumn);

    auto* layout = new QVBoxLayout;
    auto* searchBox = new QLineEdit;
    searchBox->setClearButtonEnabled(true);
    searchBox->setPlaceholderText(i18n("Search..."));
    connect(searchBox, &QLineEdit::textChanged, filterModel, &QSortFilterProxyModel::setFilterFixedString);

    tableView = new QTableView;
    tableView->setModel(filterModel);
    tableView->setSelectionMode(QTableView::SingleSelection);
    tableView->setSelectionBehavior(QTableView::SelectRows);
    tableView->setShowGrid(false);
    tableView->verticalHeader()->hide();
    connect(tableView, &QTableView::doubleClicked, this, &ChooseExistingReferenceWindow::itemSelected);

    layout->addWidget(searchBox);
    layout->addWidget(tableView);
    setLayout(layout);
}
