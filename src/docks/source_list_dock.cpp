/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "source_list_dock.h"

#include "data/data_manager.h"
#include "data/source.h"
#include "utils/formatted_identifier_delegate.h"
#include "utils/model_utils.h"
#include "utils/rich_text_plain_delegate.h"

#include <KLocalizedString>
#include <QHeaderView>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>

SourceTreeWidget::SourceTreeWidget(QWidget* parent) : QWidget(parent) {
    auto* treeModel = DataManager::get().sourcesTreeModel(this);

    // Create a searchable model.
    auto* filtered = new QSortFilterProxyModel(this);
    filtered->setSourceModel(treeModel);
    filtered->setFilterKeyColumn(SourcesTableModel::TITLE);
    filtered->setFilterCaseSensitivity(Qt::CaseInsensitive);

    auto* searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText(i18n("Search.."));
    searchBox->setClearButtonEnabled(true);

    connect(searchBox, &QLineEdit::textEdited, filtered, &QSortFilterProxyModel::setFilterFixedString);

    treeView = new QTreeView(this);
    treeView->setModel(filtered);
    treeView->setSelectionBehavior(QTreeView::SelectRows);
    treeView->setSelectionMode(QTreeView::SelectionMode::SingleSelection);
    treeView->setSortingEnabled(true);
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    treeView->header()->setSectionResizeMode(SourcesTableModel::TITLE, QHeaderView::Stretch);
    treeView->setItemDelegateForColumn(
        SourcesTableModel::ID, new FormattedIdentifierDelegate(treeView, FormattedIdentifierDelegate::SOURCE)
    );
    treeView->setItemDelegateForColumn(SourcesTableModel::NOTE, new RichTextPlainDelegate(treeView));
    treeView->hideColumn(SourcesTableModel::PARENT_ID);
    treeView->setUniformRowHeights(true);
    // treeView->expandAll();
    treeView->expandToDepth(1);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(searchBox);
    layout->addWidget(treeView);

    connect(
        treeView->selectionModel(),
        &QItemSelectionModel::selectionChanged,
        this,
        &SourceTreeWidget::handleSelectedNewRow
    );
}

void SourceTreeWidget::handleSelectedNewRow(const QItemSelection& selected) {
    if (selected.empty()) {
        return;
    }

    auto personId = getIdFromSelection(selected, treeView->model(), SourcesTableModel::ID);

    Q_EMIT handleSourceSelected(personId);
}

SourceListDock::SourceListDock() :
    DockWidget(QStringLiteral("Sources"), KDDockWidgets::DockWidgetOption_DeleteOnClose) {
    auto* treeView = new SourceTreeWidget(this);
    setWidget(treeView);

    connect(treeView, &SourceTreeWidget::handleSourceSelected, this, &SourceListDock::handleSourceSelected);
}
