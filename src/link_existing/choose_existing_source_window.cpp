/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "choose_existing_source_window.h"

#include "domain/source/source_list_model.h"
#include "utils/formatted_identifier_delegate.h"

#include <KLocalizedString>
#include <KRearrangeColumnsProxyModel>
#include <QHeaderView>
#include <QLabel>
#include <QTableView>

QVariant ChooseExistingSourceWindow::selectSource(QWidget* parent) {
    ChooseExistingSourceWindow dialog(parent);
    dialog.exec();
    return dialog.selected;
}

ChooseExistingSourceWindow::ChooseExistingSourceWindow(QWidget* parent) :
    ChooseExistingReferenceWindow(-1, SourcesListModel::ID, new SourcesListModel(parent), parent) {
    setWindowTitle(i18n("Link existing person"));
    tableHelpText->setText(i18n("Choose an existing person"));
    displayModel->setSourceColumns({SourcesListModel::ID, SourcesListModel::AUTHOR, SourcesListModel::TITLE});
    tableView->setItemDelegateForColumn(
        SourcesListModel::ID,
        new FormattedIdentifierDelegate(tableView, FormattedIdentifierDelegate::SOURCE)
    );
    tableView->horizontalHeader()->setSectionResizeMode(SourcesListModel::ID, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(SourcesListModel::AUTHOR, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}
