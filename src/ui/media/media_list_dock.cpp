/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "media_list_dock.h"

#include "domain/media/media_entities.h"
#include "domain/media/media_list_model.h"
#include "domain/media/media_repository.h"
#include "domain/media/media_service.h"
#include "ui/media/media_edit_dialog.h"
#include "utils/formatted_identifier_delegate.h"

#include <KLocalizedString>
#include <QDesktopServices>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QUrl>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

MediaTableWidget::MediaTableWidget(QWidget* parent) : QWidget(parent) {
    auto* model = new MediaListModel(this);

    auto* filtered = new QSortFilterProxyModel(this);
    filtered->setSourceModel(model);
    filtered->setFilterKeyColumn(MediaListModel::TITLE);
    filtered->setFilterCaseSensitivity(Qt::CaseInsensitive);

    auto* searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText(i18n("Search.."));
    searchBox->setClearButtonEnabled(true);
    connect(searchBox, &QLineEdit::textEdited, filtered, &QSortFilterProxyModel::setFilterFixedString);

    tableView = new QTableView(this);
    tableView->setModel(filtered);
    tableView->setSelectionBehavior(QTableView::SelectRows);
    tableView->setSelectionMode(QTableView::SingleSelection);
    tableView->setSortingEnabled(true);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->horizontalHeader()->setSectionResizeMode(MediaListModel::TITLE, QHeaderView::Stretch);
    tableView->setItemDelegateForColumn(
        MediaListModel::ID, new FormattedIdentifierDelegate(tableView, FormattedIdentifierDelegate::MEDIA)
    );
    tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(tableView, &QTableView::customContextMenuRequested, this, &MediaTableWidget::onContextMenuRequested);
    connect(tableView, &QTableView::doubleClicked, this, &MediaTableWidget::onDoubleClicked);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(searchBox);
    layout->addWidget(tableView);
}

void MediaTableWidget::openEditDialog(IntegerPrimaryKey mediaId) {
    MediaRepository repo;
    auto entity = repo.findById(mediaId);
    if (!entity.has_value()) {
        return;
    }
    MediaEditDialog dialog(*entity, this);
    dialog.exec();
}

void MediaTableWidget::onDoubleClicked(const QModelIndex& index) {
    if (!index.isValid()) {
        return;
    }
    const auto mediaId =
        index.siblingAtColumn(MediaListModel::ID).data(Qt::EditRole).value<IntegerPrimaryKey>();
    openEditDialog(mediaId);
}

void MediaTableWidget::onContextMenuRequested(const QPoint& pos) {
    const auto index = tableView->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    const auto mediaId =
        index.siblingAtColumn(MediaListModel::ID).data(Qt::EditRole).value<IntegerPrimaryKey>();

    QMenu menu(tableView);

    auto* editAction = new QAction(i18n("&Edit..."), &menu);
    editAction->setIcon(QIcon::fromTheme(u"document-edit"_s));
    connect(editAction, &QAction::triggered, this, [this, mediaId]() {
        openEditDialog(mediaId);
    });
    menu.addAction(editAction);

    auto* openAction = new QAction(i18n("&Open File"), &menu);
    openAction->setIcon(QIcon::fromTheme(u"document-open"_s));
    connect(openAction, &QAction::triggered, this, [mediaId]() {
        MediaRepository repo;
        auto entity = repo.findById(mediaId);
        if (!entity.has_value()) {
            return;
        }
        const auto absolutePath = MediaService::instance().resolveAbsolutePath(entity->path);
        QDesktopServices::openUrl(QUrl::fromLocalFile(absolutePath));
    });
    menu.addAction(openAction);

    menu.addSeparator();

    auto* deleteAction = new QAction(i18n("&Delete"), &menu);
    deleteAction->setIcon(QIcon::fromTheme(u"edit-delete"_s));
    connect(deleteAction, &QAction::triggered, this, [this, mediaId]() {
        const auto result = QMessageBox::warning(
            this,
            i18n("Delete Media"),
            i18n("Are you sure you want to delete this media record? The file on disk will not be removed."),
            QMessageBox::Yes | QMessageBox::No
        );
        if (result == QMessageBox::Yes) {
            MediaRepository repo;
            Q_UNUSED(repo.remove(mediaId));
        }
    });
    menu.addAction(deleteAction);

    menu.exec(tableView->viewport()->mapToGlobal(pos));
}

MediaListDock::MediaListDock() :
    DockWidget(u"Media"_s, KDDockWidgets::DockWidgetOption_DeleteOnClose) {
    setWidget(new MediaTableWidget(this));
}
