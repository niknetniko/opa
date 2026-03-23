/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "source_list_dock.h"

#include "domain/source/source_list_model.h"
#include "domain/source/source_repository.h"
#include "ui/source/editor/source_editor_dialog.h"
#include "utils/formatted_identifier_delegate.h"
#include "utils/model_utils.h"
#include "utils/rich_text_plain_delegate.h"
#include "utils/tree_proxy_model.h"

#include <KLocalizedString>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
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
    filtered->setRecursiveFilteringEnabled(true);

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
    treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeView, &QTreeView::customContextMenuRequested, this, &SourceTreeWidget::onContextMenuRequested);

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

using namespace Qt::StringLiterals;

void SourceTreeWidget::onContextMenuRequested(const QPoint& pos) {
    const auto index = treeView->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    const auto sourceId = index.siblingAtColumn(SourcesListModel::ID).data(Qt::EditRole).value<IntegerPrimaryKey>();

    QMenu menu(treeView);

    auto* editAction = new QAction(i18n("&Edit Source"), &menu);
    editAction->setIcon(QIcon::fromTheme(u"document-edit"_s));
    connect(editAction, &QAction::triggered, this, [this, sourceId]() {
        SourceEditorDialog::showDialogForExistingSource(sourceId, this);
    });
    menu.addAction(editAction);

    auto* deleteAction = new QAction(i18n("&Delete Source"), &menu);
    deleteAction->setIcon(QIcon::fromTheme(u"edit-delete"_s));
    connect(deleteAction, &QAction::triggered, this, [this, sourceId]() {
        const auto result = QMessageBox::warning(
            this,
            i18n("Delete Source"),
            i18n("Are you sure you want to delete this source?"),
            QMessageBox::Yes | QMessageBox::No
        );
        if (result == QMessageBox::Yes) {
            SourceRepository repo;
            Q_UNUSED(repo.remove(sourceId));
        }
    });
    menu.addAction(deleteAction);

    menu.exec(treeView->viewport()->mapToGlobal(pos));
}

SourceListDock::SourceListDock() :
    DockWidget(QStringLiteral("Sources"), KDDockWidgets::DockWidgetOption_DeleteOnClose) {
    auto* treeView = new SourceTreeWidget(this);
    setWidget(treeView);
    connect(treeView, &SourceTreeWidget::handleSourceSelected, this, &SourceListDock::handleSourceSelected);
}
