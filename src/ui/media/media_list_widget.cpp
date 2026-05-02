/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "media_list_widget.h"

#include "domain/media/media_list_model.h"
#include "domain/media/media_repository.h"
#include "domain/media/media_service.h"
#include "link_existing/choose_existing_media_window.h"
#include "media_edit_dialog.h"
#include "utils/formatted_identifier_delegate.h"

#include <KLocalizedString>
#include <QAction>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QMimeDatabase>
#include <QTableView>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

MediaListWidget::MediaListWidget(LoadFn load, AttachFn attach, DetachFn detach, QWidget* parent) :
    QWidget(parent),
    loadFn(std::move(load)),
    attachFn(std::move(attach)),
    detachFn(std::move(detach)) {

    auto* layout = new QVBoxLayout(this);

    model = new ObjectTableModel<MediaEntity>(this);
    model->addColumn(i18n("ID"), &MediaEntity::id);
    model->addColumn(i18n("Title"), [](const MediaEntity& e) -> QVariant {
        return e.title.value_or(QFileInfo(e.path).fileName());
    });
    model->addColumn(i18n("File"), &MediaEntity::path);
    model->addColumn(i18n("Type"), &MediaEntity::mimeType);

    auto* toolbar = new QToolBar(this);

    addAction = new QAction(toolbar);
    addAction->setText(i18n("Add file…"));
    addAction->setIcon(QIcon::fromTheme(u"list-add"_s));
    toolbar->addAction(addAction);

    linkAction = new QAction(toolbar);
    linkAction->setText(i18n("Link existing…"));
    linkAction->setIcon(QIcon::fromTheme(u"insert-link"_s));
    toolbar->addAction(linkAction);

    toolbar->addSeparator();

    editAction = new QAction(toolbar);
    editAction->setText(i18n("Edit…"));
    editAction->setIcon(QIcon::fromTheme(u"document-edit"_s));
    editAction->setEnabled(false);
    toolbar->addAction(editAction);

    openAction = new QAction(toolbar);
    openAction->setText(i18n("Open"));
    openAction->setIcon(QIcon::fromTheme(u"document-open"_s));
    openAction->setEnabled(false);
    toolbar->addAction(openAction);

    removeAction = new QAction(toolbar);
    removeAction->setText(i18n("Remove"));
    removeAction->setIcon(QIcon::fromTheme(u"edit-delete"_s));
    removeAction->setEnabled(false);
    toolbar->addAction(removeAction);

    layout->addWidget(toolbar);

    tableView = new QTableView(this);
    tableView->setModel(model);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->verticalHeader()->hide();
    tableView->setItemDelegateForColumn(
        MediaListModel::ID,
        new FormattedIdentifierDelegate(tableView, FormattedIdentifierDelegate::MEDIA)
    );
    layout->addWidget(tableView);

    connect(addAction, &QAction::triggered, this, &MediaListWidget::onAddFile);
    connect(linkAction, &QAction::triggered, this, &MediaListWidget::onLinkExisting);
    connect(editAction, &QAction::triggered, this, &MediaListWidget::onEdit);
    connect(openAction, &QAction::triggered, this, &MediaListWidget::onOpen);
    connect(removeAction, &QAction::triggered, this, &MediaListWidget::onRemove);
    connect(tableView, &QTableView::doubleClicked, this, [this] { onEdit(); });
    connect(tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MediaListWidget::updateButtons);

    reload();
}

void MediaListWidget::reload() {
    model->setItems(loadFn());
    updateButtons();
}

void MediaListWidget::onAddFile() {
    const auto path = QFileDialog::getOpenFileName(this, i18n("Select File to Attach"));
    if (path.isEmpty()) {
        return;
    }

    if (!MediaService::isActive()) {
        return;
    }

    const auto relativePath = MediaService::instance().importFile(path);
    if (!relativePath) {
        return;
    }

    const QFileInfo info(path);
    const std::optional<QString> title = info.completeBaseName();
    const QString mimeType = QMimeDatabase().mimeTypeForFile(path).name();

    MediaRepository repo;
    const auto mediaId = repo.insert(*relativePath, title, std::nullopt, mimeType);
    if (mediaId) {
        Q_UNUSED(attachFn(*mediaId));
        reload();
    }
}

void MediaListWidget::onLinkExisting() {
    const auto result = ChooseExistingMediaWindow::selectMedia(this);
    if (!result.isValid()) {
        return;
    }
    Q_UNUSED(attachFn(result.toLongLong()));
    reload();
}

void MediaListWidget::onEdit() {
    const auto indexes = tableView->selectionModel()->selectedRows();
    if (indexes.isEmpty()) {
        return;
    }

    const auto& item = model->getItems().at(indexes.constFirst().row());
    auto* dialog = new MediaEditDialog(item, this);
    connect(dialog, &QDialog::accepted, this, &MediaListWidget::reload);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->open();
}

void MediaListWidget::onOpen() const {
    const auto indexes = tableView->selectionModel()->selectedRows();
    if (indexes.isEmpty()) {
        return;
    }

    const auto& item = model->getItems().at(indexes.constFirst().row());
    if (!MediaService::isActive()) {
        return;
    }

    const QString absolutePath = MediaService::instance().resolveAbsolutePath(item.path);
    QDesktopServices::openUrl(QUrl::fromLocalFile(absolutePath));
}

void MediaListWidget::onRemove() {
    const auto indexes = tableView->selectionModel()->selectedRows();
    if (indexes.isEmpty()) {
        return;
    }

    const auto mediaId = model->getItems().at(indexes.constFirst().row()).id;
    Q_UNUSED(detachFn(mediaId));
    reload();
}

void MediaListWidget::updateButtons() const {
    const bool hasSelection = tableView->selectionModel()->hasSelection();
    editAction->setEnabled(hasSelection);
    openAction->setEnabled(hasSelection);
    removeAction->setEnabled(hasSelection);
}
