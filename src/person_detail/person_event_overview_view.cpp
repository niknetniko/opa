/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "person_event_overview_view.h"

#include "data/data_manager.h"
#include "data/event.h"
#include "events/event_editor.h"
#include "link_existing/choose_existing_event_window.h"
#include "utils/formatted_identifier_delegate.h"

#include <QAbstractButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVBoxLayout>

EventsOverviewView::EventsOverviewView(IntegerPrimaryKey personId, QWidget* parent) : QWidget(parent) {
    this->personId = personId;
    this->baseModel = DataManager::get().eventsModelForPerson(this, personId);

    treeView = new QTreeView(this);
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    treeView->setUniformRowHeights(true);
    treeView->setRootIsDecorated(true);

    treeView->setModel(baseModel);
    treeView->hideColumn(PersonEventsModel::ROLE_ID);
    treeView->setItemDelegateForColumn(
        PersonEventsModel::ID, new FormattedIdentifierDelegate(treeView, FormattedIdentifierDelegate::EVENT)
    );
    treeView->header()->setSortIndicatorClearable(false);
    treeView->header()->setSectionResizeMode(PersonEventsModel::TYPE, QHeaderView::ResizeToContents);
    treeView->header()->setSectionResizeMode(PersonEventsModel::ID, QHeaderView::ResizeToContents);
    treeView->header()->setSectionResizeMode(PersonEventsModel::DATE, QHeaderView::ResizeToContents);
    treeView->expandAll();

    // Wrap in a VBOX for layout reasons.
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(treeView);

    connect(
        treeView->selectionModel(),
        &QItemSelectionModel::selectionChanged,
        this,
        &EventsOverviewView::handleSelectedNewRow
    );
    // Support models being reset.
    connect(treeView->model(), &QAbstractItemModel::modelReset, this, [this] {
        this->handleSelectedNewRow(QItemSelection(), QItemSelection());
    });

    connect(treeView, &QTreeView::doubleClicked, this, &EventsOverviewView::handleDoubleClick);
}

void EventsOverviewView::handleSelectedNewRow(
    const QItemSelection& selected, [[maybe_unused]] const QItemSelection& deselected
) {
    Q_EMIT this->selectedEvent(this->treeView->selectionModel()->model(), selected);
}

void EventsOverviewView::handleDoubleClick([[maybe_unused]] const QModelIndex& clicked) {
    this->editSelectedEvent();
}

void EventsOverviewView::handleNewEvent() {
    // First, look up the ID for the "default" event role just to be sure.
    auto defaultRoleId = EventRolesModel::getDefaultRole();
    if (!defaultRoleId.isValid()) {
        qWarning() << "Default role not found, aborting new event.";
        return;
    }

    // Also lookup the ID for a first event.
    // TODO: do this intelligently.
    auto* typeModel = DataManager::get().eventTypesModel();
    auto defaultType = EventTypes::nameOriginToString[EventTypes::Birth].toString();
    auto defaultEventTypeIndex =
        typeModel->match(typeModel->index(0, EventTypesModel::TYPE), Qt::DisplayRole, defaultType);

    if (!defaultEventTypeIndex.first().isValid()) {
        qWarning() << "Default type not found, aborting new event.";
        return;
    }
    auto defaultTypeId = typeModel->index(defaultEventTypeIndex.first().row(), EventTypesModel::ID).data();

    if (!QSqlDatabase::database().transaction()) {
        qWarning() << "Could not get transaction on database for some reason.";
        qDebug() << QSqlDatabase::database().lastError();
        return;
    }

    // Second, insert a new event.
    auto* eventModel = DataManager::get().eventsModel();
    auto newEventRecord = eventModel->record();
    newEventRecord.setGenerated(EventsModel::ID, false);
    newEventRecord.setValue(EventsModel::TYPE_ID, defaultTypeId);

    if (!eventModel->insertRecord(-1, newEventRecord)) {
        QMessageBox::warning(this, tr("Could not insert event"), tr("Problem inserting new event into database."));
        qDebug() << "Could not get last inserted ID for some reason:";
        qDebug() << eventModel->lastError();
        return;
    }

    auto newEventId = eventModel->query().lastInsertId();
    if (!newEventId.isValid()) {
        QMessageBox::warning(this, tr("Could not insert event"), tr("Problem inserting new event into database."));
        qDebug() << "Could not get last inserted ID for some reason:";
        qDebug() << eventModel->lastError();
        QSqlDatabase::database().rollback();
        return;
    }

    // Finally, link the event to our person.
    auto* eventRelationModel = DataManager::get().eventRelationsModel();
    auto eventRelationRecord = eventRelationModel->record();
    eventRelationRecord.setValue(EventRelationsModel::EVENT_ID, newEventId);
    eventRelationRecord.setValue(EventRelationsModel::PERSON_ID, this->personId);
    eventRelationRecord.setValue(EventRelationsModel::ROLE_ID, defaultRoleId);

    if (!eventRelationModel->insertRecord(-1, eventRelationRecord)) {
        QMessageBox::warning(
            this, tr("Could not event relation"), tr("Problem inserting new event relation into database.")
        );
        qDebug() << "Could not insert event relation for some reason:";
        qDebug() << eventRelationModel->lastError();
        QSqlDatabase::database().rollback();
        return;
    }

    if (!QSqlDatabase::database().commit()) {
        qWarning() << "Could not commit transaction on database for some reason.";
        qDebug() << QSqlDatabase::database().lastError();
        return;
    }

    auto* singleEventModel = DataManager::get().singleEventModel(this, newEventId);
    auto* singleRelationModel =
        DataManager::get().singleEventRelationModel(this, newEventId, defaultRoleId, this->personId);

    auto* editorWindow = new EventEditor(singleRelationModel, singleEventModel, true, this);
    editorWindow->show();
}

void EventsOverviewView::editSelectedEvent() {
    // Get the currently selected name.
    auto* const selection = this->treeView->selectionModel();
    if (!selection->hasSelection()) {
        return;
    }

    const auto selectRow = selection->selectedRows().first();

    auto eventId = this->treeView->model()->index(selectRow.row(), PersonEventsModel::ID, selectRow.parent()).data();
    auto eventRoleId =
        this->treeView->model()->index(selectRow.row(), PersonEventsModel::ROLE_ID, selectRow.parent()).data();

    auto* eventModel = DataManager::get().singleEventModel(this, eventId);
    auto* eventRelationModel =
        DataManager::get().singleEventRelationModel(this, eventId, eventRoleId, QVariant::fromValue(personId));

    auto* editorWindow = new EventEditor(eventRelationModel, eventModel, false, this);
    editorWindow->show();
}

void EventsOverviewView::removeSelectedEvent() const {
    auto* selection = this->treeView->selectionModel();
    if (!selection->hasSelection()) {
        qDebug() << "There is no selection, so not deleting anything.";
        return;
    }

    qDebug() << "Selection itself is " << selection;

    auto selectRow = selection->selectedIndexes().first();
    auto eventId = treeView->model()->index(selectRow.row(), PersonEventsModel::ID, selectRow.parent()).data();

    // Look up where it is linked.
    auto* relationModel = DataManager::get().eventRelationsModel();
    auto usedCount =
        relationModel->match(relationModel->index(0, EventRelationsModel::EVENT_ID), Qt::DisplayRole, eventId, -1)
            .size();

    QMessageBox confirmationBox;
    confirmationBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    confirmationBox.setText(i18n("Delete this event?"));
    confirmationBox.button(QMessageBox::Ok)->setText(i18n("Delete event"));
    confirmationBox.setIcon(QMessageBox::Question);
    if (usedCount == 1) {
        confirmationBox.setInformativeText(i18n("This event is not linked to anybody else."));
        confirmationBox.setDefaultButton(QMessageBox::Ok);
    } else {
        confirmationBox.setInformativeText(
            i18np("This event is linked to one other person", "This event is linked to %1 other people", usedCount)
        );
        confirmationBox.setDefaultButton(QMessageBox::Cancel);
    }

    if (confirmationBox.exec() != QMessageBox::Ok) {
        return;
    }

    qDebug() << "Will delete event with ID " << eventId.toLongLong();

    // Find the row in the original model.
    auto* eventsModel = DataManager::get().eventsModel();
    auto result = eventsModel->match(eventsModel->index(0, EventsModel::ID), Qt::DisplayRole, eventId).constFirst();
    if (!eventsModel->removeRow(result.row())) {
        qWarning() << "Could not delete event" << eventId.toLongLong();
    }
    eventsModel->select();
}

void EventsOverviewView::unlinkSelectedEvent() {
    auto* selection = this->treeView->selectionModel();
    if (!selection->hasSelection()) {
        qDebug() << "There is no selection, so not deleting anything.";
        return;
    }

    qDebug() << "Selection itself is " << selection;

    auto selectRow = selection->selectedIndexes().first();
    auto eventId = treeView->model()->index(selectRow.row(), PersonEventsModel::ID, selectRow.parent()).data();
    auto roleId = treeView->model()->index(selectRow.row(), PersonEventsModel::ROLE_ID, selectRow.parent()).data();

    QMessageBox confirmationBox;
    confirmationBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    confirmationBox.setText(i18n("Unlink this event?"));
    confirmationBox.setInformativeText(
        i18n("This will remove the event from this person, but the event itself will not be deleted.")
    );
    confirmationBox.button(QMessageBox::Ok)->setText(i18n("Unlink event"));
    confirmationBox.setIcon(QMessageBox::Question);
    confirmationBox.setDefaultButton(QMessageBox::Ok);

    if (confirmationBox.exec() != QMessageBox::Ok) {
        return;
    }

    auto* relationModel = DataManager::get().eventRelationsModel();
    auto* matchedModel = DataManager::get().singleEventRelationModel(this, eventId, roleId, personId);
    auto originalRow = mapToSourceModel(matchedModel->index(0, 0));
    qDebug() << "Mapped to row in original model" << originalRow.row();

    if (!relationModel->removeRow(originalRow.row())) {
        qWarning() << "Could not unlink event" << eventId.toLongLong();
    }
    relationModel->select();
}

void EventsOverviewView::linkExistingEvent() {
    // TODO: allow setting the role of this link!
    const auto selectedEvent = ChooseExistingEventWindow::selectEventAndRole(this);
    if (!selectedEvent.isValid()) {
        return;
    }

    auto* eventRelationModel = DataManager::get().eventRelationsModel();
    auto eventRelationRecord = eventRelationModel->record();
    eventRelationRecord.setValue(EventRelationsModel::EVENT_ID, selectedEvent.eventId);
    eventRelationRecord.setValue(EventRelationsModel::PERSON_ID, this->personId);
    eventRelationRecord.setValue(EventRelationsModel::ROLE_ID, selectedEvent.roleId);

    if (!eventRelationModel->insertRecord(-1, eventRelationRecord)) {
        QMessageBox::warning(
            this, tr("Could not event relation"), tr("Problem inserting new event relation into database.")
        );
        qDebug() << "Could not insert event relation for some reason:";
        qDebug() << eventRelationModel->lastError();
    }
}
