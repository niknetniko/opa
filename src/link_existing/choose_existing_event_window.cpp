
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "choose_existing_event_window.h"

#include <data/data_manager.h>
#include <data/event.h>
#include <utils/formatted_identifier_delegate.h>

#include <KRearrangeColumnsProxyModel>
#include <QDebug>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QTableView>

QDebug operator<<(QDebug dbg, const ExistingEventSelection& selection) {
    const QDebugStateSaver saver(dbg);
    dbg.resetFormat();
    dbg << "ExistingEventSelection{ .eventId=" << selection.eventId << ", .roleId=" << selection.roleId << "}";
    return dbg;
}

ExistingEventSelection ChooseExistingEventWindow::selectEventAndRole(QWidget* parent) {
    ChooseExistingEventWindow dialog(parent);
    dialog.exec();

    return {.eventId = dialog.selected, .roleId = dialog.selectedRole};
}

void ChooseExistingEventWindow::accept() {
    if (eventRoleComboBox->currentIndex() != -1) {
        selectedRole = eventRoleComboBox->model()->index(eventRoleComboBox->currentIndex(), EventRolesModel::ID).data();
    }
    ChooseExistingReferenceWindow::accept();
}

ChooseExistingEventWindow::ChooseExistingEventWindow(QWidget* parent) :
    ChooseExistingReferenceWindow(-1, EventsModel::ID, DataManager::get().eventsModel(), parent) {

    // Set some stuff for the parent.
    setWindowTitle(i18n("Link existing event"));
    tableHelpText->setText(i18n("Choose an existing event to link to the person"));
    displayModel->setSourceColumns({EventsModel::ID, EventsModel::TYPE, EventsModel::DATE, EventsModel::NAME});
    tableView->setItemDelegateForColumn(
        EventsModel::ID, new FormattedIdentifierDelegate(tableView, FormattedIdentifierDelegate::EVENT)
    );
    tableView->horizontalHeader()->setSectionResizeMode(EventsModel::ID, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(EventsModel::DATE, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Add a new group box with the role in it.
    auto* roleGroupBox = new QGroupBox(this);
    roleGroupBox->setFlat(true);
    roleGroupBox->setTitle(i18n("Relation options"));
    roleGroupBox->setAlignment(Qt::AlignLeft);
    auto* innerLayout = new QVBoxLayout(roleGroupBox);
    auto* formLayout = new QFormLayout;

    auto* roleHelpText = new QLabel(roleGroupBox);
    roleHelpText->setText(i18n("Set options for the relationship between the event and the person, such as their role.")
    );

    innerLayout->addWidget(roleHelpText);
    innerLayout->addLayout(formLayout);

    auto* eventRoleLabel = new QLabel(roleGroupBox);
    eventRoleLabel->setText(i18n("Role"));
    formLayout->setWidget(0, QFormLayout::LabelRole, eventRoleLabel);

    auto* comboBoxModel = DataManager::get().eventRolesModel();
    auto defaultRoleId = EventRolesModel::getDefaultRole();
    if (!defaultRoleId.isValid()) {
        qWarning() << "Default role not found, aborting new event.";
        return;
    }
    auto defaultRoleIndex =
        comboBoxModel->match(comboBoxModel->index(0, EventRolesModel::ID), Qt::DisplayRole, defaultRoleId).constFirst();

    eventRoleComboBox = new QComboBox(roleGroupBox);
    eventRoleComboBox->setEditable(true);
    eventRoleComboBox->setModel(comboBoxModel);
    eventRoleComboBox->setModelColumn(EventRolesModel::ROLE);
    eventRoleComboBox->setCurrentIndex(defaultRoleIndex.row());
    formLayout->setWidget(0, QFormLayout::FieldRole, eventRoleComboBox);

    layout->addWidget(roleGroupBox);
}
