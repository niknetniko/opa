/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "choose_existing_location_window.h"

#include "domain/location/location_list_model.h"
#include "utils/formatted_identifier_delegate.h"

#include <KLocalizedString>
#include <KRearrangeColumnsProxyModel>
#include <QHeaderView>
#include <QLabel>
#include <QTableView>

QVariant ChooseExistingLocationWindow::selectLocation(QWidget* parent) {
    ChooseExistingLocationWindow dialog(parent);
    dialog.exec();
    return dialog.selected;
}

ChooseExistingLocationWindow::ChooseExistingLocationWindow(QWidget* parent) :
    ChooseExistingReferenceWindow(-1, LocationListModel::ID, new LocationListModel(parent), parent) {
    setWindowTitle(i18n("Select location"));
    tableHelpText->setText(i18n("Choose an existing location"));
    displayModel->setSourceColumns({LocationListModel::ID, LocationListModel::NAME});
    tableView->setItemDelegateForColumn(
        LocationListModel::ID, new FormattedIdentifierDelegate(tableView, FormattedIdentifierDelegate::LOCATION)
    );
    tableView->horizontalHeader()->setSectionResizeMode(LocationListModel::ID, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}
