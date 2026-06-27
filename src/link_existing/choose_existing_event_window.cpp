/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "choose_existing_event_window.h"

#include "domain/event/event_list_model.h"
#include "domain/event/event_repository.h"
#include "domain/event/event_roles_model.h"
#include "utils/formatted_identifier_delegate.h"

#include <KLocalizedString>
#include <KRearrangeColumnsProxyModel>
#include <QComboBox>
#include <QDebug>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QTableView>

bool ExistingEventSelection::isValid() const {
    return eventId.isValid() && roleId.isValid();
}

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
        selectedRole =
            eventRoleComboBox->model()->index(eventRoleComboBox->currentIndex(), EventRolesListModel::ID).data();
    }
    ChooseExistingReferenceWindow::accept();
}

ChooseExistingEventWindow::ChooseExistingEventWindow(QWidget* parent) :
    ChooseExistingReferenceWindow(-1, EventListModel::ID, new EventListModel(nullptr), parent) {

    // Set some stuff for the parent.
    setWindowTitle(i18n("Link existing event"));
    tableHelpText->setText(i18n("Choose an existing event to link to the person"));
    displayModel->setSourceColumns(
        {EventListModel::ID, EventListModel::TYPE, EventListModel::DATE, EventListModel::NAME}
    );
    tableView->setItemDelegateForColumn(
        EventListModel::ID,
        new FormattedIdentifierDelegate(tableView, FormattedIdentifierDelegate::EVENT)
    );
    tableView->horizontalHeader()->setSectionResizeMode(EventListModel::ID, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(EventListModel::DATE, QHeaderView::ResizeToContents);
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
    roleHelpText->setText(
        i18n("Set options for the relationship between the event and the person, such as their role.")
    );

    innerLayout->addWidget(roleHelpText);
    innerLayout->addLayout(formLayout);

    auto* eventRoleLabel = new QLabel(roleGroupBox);
    eventRoleLabel->setText(i18n("Role"));
    formLayout->setWidget(0, QFormLayout::LabelRole, eventRoleLabel);

    auto* comboBoxModel = new EventRolesListModel(this);
    EventRepository repo;
    auto defaultRoleId = repo.findEventRoleIdByName(QStringLiteral("Primary"));
    int defaultRoleRow = 0;
    if (defaultRoleId.has_value()) {
        auto defaultRoleIndex =
            comboBoxModel->match(comboBoxModel->index(0, EventRolesListModel::ID), Qt::DisplayRole, *defaultRoleId);
        if (!defaultRoleIndex.isEmpty()) {
            defaultRoleRow = defaultRoleIndex.constFirst().row();
        }
    }

    eventRoleComboBox = new QComboBox(roleGroupBox);
    eventRoleComboBox->setEditable(true);
    eventRoleComboBox->setModel(comboBoxModel);
    eventRoleComboBox->setModelColumn(EventRolesListModel::ROLE);
    eventRoleComboBox->setCurrentIndex(defaultRoleRow);
    formLayout->setWidget(0, QFormLayout::FieldRole, eventRoleComboBox);

    layout->addWidget(roleGroupBox);
}
