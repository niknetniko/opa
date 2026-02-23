
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "choose_existing_person_window.h"

#include "domain/person/person_display_model.h"
#include "utils/formatted_identifier_delegate.h"

#include <KLocalizedString>
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
    ChooseExistingReferenceWindow(-1, PersonDisplayModel::ID, new PersonDisplayModel(parent), parent) {
    setWindowTitle(i18n("Link existing person"));
    tableHelpText->setText(i18n("Choose an existing person"));
    displayModel->setSourceColumns({PersonDisplayModel::ID, PersonDisplayModel::NAME});

    tableView->setItemDelegateForColumn(
        PersonDisplayModel::ID, new FormattedIdentifierDelegate(tableView, FormattedIdentifierDelegate::PERSON)
    );
    tableView->horizontalHeader()->setSectionResizeMode(PersonDisplayModel::ID, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(PersonDisplayModel::NAME, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}
