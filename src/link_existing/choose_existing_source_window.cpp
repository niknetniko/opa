
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "choose_existing_source_window.h"

#include "data/data_manager.h"
#include "data/source.h"
#include "utils/formatted_identifier_delegate.h"

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
    ChooseExistingReferenceWindow(-1, SourcesTableModel::ID, DataManager::get().sourcesModel(), parent) {
    setWindowTitle(i18n("Link existing person"));
    tableHelpText->setText(i18n("Choose an existing person"));
    displayModel->setSourceColumns({SourcesTableModel::ID, SourcesTableModel::AUTHOR, SourcesTableModel::TITLE});

    tableView->setItemDelegateForColumn(
        SourcesTableModel::ID, new FormattedIdentifierDelegate(tableView, FormattedIdentifierDelegate::SOURCE)
    );
    tableView->horizontalHeader()->setSectionResizeMode(SourcesTableModel::ID, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(SourcesTableModel::AUTHOR, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}
