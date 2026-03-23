/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "citation_list_widget.h"

#include "link_existing/choose_existing_source_window.h"
#include "ui/source/editor/source_editor_dialog.h"

#include <KLocalizedString>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

CitationListWidget::CitationListWidget(LoadFn load, AddFn add, RemoveFn remove, QWidget* parent) :
    QWidget(parent),
    loadFn(std::move(load)),
    addFn(std::move(add)),
    removeFn(std::move(remove)) {

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    model = new ObjectTableModel<SourceEntity>(this);
    model->addColumn(i18n("ID"), &SourceEntity::id);
    model->addColumn(i18n("Title"), &SourceEntity::title);
    model->addColumn(i18n("Type"), &SourceEntity::type);
    model->addColumn(i18n("Confidence"), &SourceEntity::confidence);

    tableView = new QTableView(this);
    tableView->setModel(model);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->verticalHeader()->hide();
    tableView->setColumnHidden(0, true);
    layout->addWidget(tableView);

    auto* buttonLayout = new QHBoxLayout;
    addNewButton = new QPushButton(i18n("Add new source"), this);
    linkExistingButton = new QPushButton(i18n("Link existing source"), this);
    removeButton = new QPushButton(i18n("Remove"), this);
    removeButton->setEnabled(false);

    buttonLayout->addWidget(addNewButton);
    buttonLayout->addWidget(linkExistingButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(removeButton);
    layout->addLayout(buttonLayout);

    connect(addNewButton, &QPushButton::clicked, this, &CitationListWidget::onAddNew);
    connect(linkExistingButton, &QPushButton::clicked, this, &CitationListWidget::onLinkExisting);
    connect(removeButton, &QPushButton::clicked, this, &CitationListWidget::onRemove);
    connect(
        tableView->selectionModel(),
        &QItemSelectionModel::selectionChanged,
        this,
        &CitationListWidget::updateRemoveButton
    );

    reload();
}

void CitationListWidget::reload() {
    model->setItems(loadFn());
    updateRemoveButton();
}

void CitationListWidget::onAddNew() {
    auto result = SourceEditorDialog::showDialogForNewSource(this);
    if (result.isValid()) {
        auto sourceId = result.toLongLong();
        Q_UNUSED(addFn(sourceId));
        reload();
    }
}

void CitationListWidget::onLinkExisting() {
    auto result = ChooseExistingSourceWindow::selectSource(this);
    if (result.isValid()) {
        auto sourceId = result.toLongLong();
        Q_UNUSED(addFn(sourceId));
        reload();
    }
}

void CitationListWidget::onRemove() {
    auto indexes = tableView->selectionModel()->selectedRows();
    if (indexes.isEmpty()) {
        return;
    }

    auto row = indexes.constFirst().row();
    auto sourceId = model->getItems().at(row).id;
    Q_UNUSED(removeFn(sourceId));
    reload();
}

void CitationListWidget::updateRemoveButton() const {
    removeButton->setEnabled(tableView->selectionModel()->hasSelection());
}
