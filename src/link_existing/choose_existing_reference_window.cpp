
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "choose_existing_reference_window.h"

#include "person_detail/person_event_tab.h"
#include "utils/model_utils.h"

#include <KLocalizedString>
#include <KRearrangeColumnsProxyModel>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVBoxLayout>

void ChooseExistingReferenceWindow::accept() {
    if (auto selection = tableView->selectionModel()->selection(); !selection.isEmpty()) {
        const auto tableViewSelected = selection.indexes().first();
        const auto sourceSelection = mapToSourceModel(tableViewSelected);
        selected = sourceModel->index(sourceSelection.row(), resultColumn).data();
    }

    QDialog::accept();
}

void ChooseExistingReferenceWindow::itemSelected(const QModelIndex& selected) {
    if (!selected.isValid()) {
        buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
        return;
    }

    accept();
}

void ChooseExistingReferenceWindow::selectionChanged(
    const QItemSelection& selected, [[maybe_unused]] const QItemSelection& deselected
) const {
    buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(!selected.indexes().isEmpty());
}

ChooseExistingReferenceWindow::ChooseExistingReferenceWindow(
    int searchColumn, int resultColumn, QAbstractItemModel* sourceModel, QWidget* parent
) :
    QDialog(parent),
    searchColumn(searchColumn),
    resultColumn(resultColumn),
    sourceModel(sourceModel) {
    Q_ASSERT(
        sourceModel->checkIndex(sourceModel->index(0, resultColumn), QAbstractItemModel::CheckIndexOption::IndexIsValid)
    );
    Q_ASSERT(
        searchColumn == -1 ||
        sourceModel->checkIndex(sourceModel->index(0, searchColumn), QAbstractItemModel::CheckIndexOption::IndexIsValid)
    );

    setWindowModality(Qt::WindowModal);

    auto* filterModel = new QSortFilterProxyModel(this);
    filterModel->setSourceModel(sourceModel);
    filterModel->setFilterKeyColumn(searchColumn);

    displayModel = new KRearrangeColumnsProxyModel(this);
    displayModel->setSourceModel(filterModel);

    layout = new QVBoxLayout;

    tableBox = new QGroupBox(this);
    tableBox->setFlat(true);
    layout->addWidget(tableBox);
    auto* innerGroupBoxLayout = new QVBoxLayout(tableBox);

    tableHelpText = new QLabel(tableBox);

    auto* searchBox = new QLineEdit(tableBox);
    searchBox->setClearButtonEnabled(true);
    searchBox->setPlaceholderText(i18n("Search..."));
    connect(searchBox, &QLineEdit::textChanged, filterModel, &QSortFilterProxyModel::setFilterFixedString);

    tableView = new QTableView(tableBox);
    tableView->setModel(displayModel);
    tableView->setSelectionMode(QTableView::SingleSelection);
    tableView->setSelectionBehavior(QTableView::SelectRows);
    tableView->setShowGrid(false);
    tableView->verticalHeader()->hide();
    connect(tableView, &QTableView::doubleClicked, this, &ChooseExistingReferenceWindow::itemSelected);
    connect(
        tableView->selectionModel(),
        &QItemSelectionModel::selectionChanged,
        this,
        &ChooseExistingReferenceWindow::selectionChanged
    );

    innerGroupBoxLayout->addWidget(tableHelpText);
    innerGroupBoxLayout->addWidget(searchBox);
    innerGroupBoxLayout->addWidget(tableView);

    buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::StandardButton::Cancel | QDialogButtonBox::StandardButton::Ok);
    buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* parentLayout = new QVBoxLayout;
    parentLayout->addLayout(layout);
    parentLayout->addWidget(buttonBox);

    setLayout(parentLayout);
}
