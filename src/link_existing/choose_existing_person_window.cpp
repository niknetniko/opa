
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "choose_existing_person_window.h"

#include "data/data_manager.h"
#include "data/names.h"
#include "data/person.h"
#include "utils/formatted_identifier_delegate.h"

#include <KRearrangeColumnsProxyModel>
#include <QHeaderView>
#include <QLabel>
#include <QTableView>

QVariant ChooseExistingPersonWindow::selectPerson(QWidget* parent) {
    ChooseExistingPersonWindow dialog(parent);
    dialog.exec();

    return dialog.selected;
}

ChooseExistingPersonWindow::ChooseExistingPersonWindow(QWidget* parent) :
    ChooseExistingReferenceWindow(-1, DisplayNameModel::ID, DataManager::get().primaryNamesModel(parent), parent) {
    setWindowTitle(i18n("Link existing person"));
    tableHelpText->setText(i18n("Choose an existing person"));
    displayModel->setSourceColumns({DisplayNameModel::ID, DisplayNameModel::NAME});

    tableView->setItemDelegateForColumn(
        DisplayNameModel::ID, new FormattedIdentifierDelegate(tableView, FormattedIdentifierDelegate::PERSON)
    );
    tableView->horizontalHeader()->setSectionResizeMode(DisplayNameModel::ID, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(DisplayNameModel::NAME, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}
