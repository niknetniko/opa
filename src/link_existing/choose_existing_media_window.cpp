/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "choose_existing_media_window.h"

#include "domain/media/media_list_model.h"
#include "utils/formatted_identifier_delegate.h"

#include <KLocalizedString>
#include <KRearrangeColumnsProxyModel>
#include <QHeaderView>
#include <QLabel>
#include <QTableView>

QVariant ChooseExistingMediaWindow::selectMedia(QWidget* parent) {
    ChooseExistingMediaWindow dialog(parent);
    dialog.exec();
    return dialog.selected;
}

ChooseExistingMediaWindow::ChooseExistingMediaWindow(QWidget* parent) :
    ChooseExistingReferenceWindow(MediaListModel::TITLE, MediaListModel::ID, new MediaListModel(parent), parent) {
    setWindowTitle(i18n("Link existing media"));
    tableHelpText->setText(i18n("Choose an existing media file"));
    displayModel->setSourceColumns(
        {MediaListModel::ID, MediaListModel::TITLE, MediaListModel::PATH, MediaListModel::MIME_TYPE}
    );
    tableView->setItemDelegateForColumn(
        MediaListModel::ID,
        new FormattedIdentifierDelegate(tableView, FormattedIdentifierDelegate::MEDIA)
    );
    tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}
