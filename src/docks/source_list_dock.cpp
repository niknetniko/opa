/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "source_list_dock.h"

#include "domain/source/source_list_model.h"
#include "utils/formatted_identifier_delegate.h"
#include "utils/model_utils.h"
#include "utils/rich_text_plain_delegate.h"
#include "utils/tree_proxy_model.h"

#include <KLocalizedString>
#include <QHeaderView>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>

SourceTreeWidget::SourceTreeWidget(QWidget* parent) : QWidget(parent) {
    auto* sourcesModel = new SourcesListModel(this);

    auto* treeModel = new TreeProxyModel(this);
    treeModel->setSourceModel(sourcesModel);
    treeModel->setIdColumn(SourcesListModel::ID);
    treeModel->setParentIdColumn(SourcesListModel::PARENT_ID);

    // Create a searchable model.
    auto* filtered = new QSortFilterProxyModel(this);
    filtered->setSourceModel(treeModel);
    filtered->setFilterKeyColumn(SourcesListModel::TITLE);
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
    treeView->header()->setSectionResizeMode(SourcesListModel::TITLE, QHeaderView::Stretch);
    treeView->setItemDelegateForColumn(
        SourcesListModel::ID, new FormattedIdentifierDelegate(treeView, FormattedIdentifierDelegate::SOURCE)
    );
    treeView->setItemDelegateForColumn(SourcesListModel::NOTE, new RichTextPlainDelegate(treeView));
    treeView->hideColumn(SourcesListModel::PARENT_ID);
    treeView->setUniformRowHeights(true);
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
    auto personId = getIdFromSelection(selected, treeView->model(), SourcesListModel::ID);
    Q_EMIT handleSourceSelected(personId);
}

SourceListDock::SourceListDock() :
    DockWidget(QStringLiteral("Sources"), KDDockWidgets::DockWidgetOption_DeleteOnClose) {
    auto* treeView = new SourceTreeWidget(this);
    setWidget(treeView);
    connect(treeView, &SourceTreeWidget::handleSourceSelected, this, &SourceListDock::handleSourceSelected);
}
