#include <QHeaderView>

#include "person_event_overview_view.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <events/event_editor.h>

#include "data/data_manager.h"
#include "data/event.h"
#include "utils/formatted_identifier_delegate.h"

EventsOverviewView::EventsOverviewView(IntegerPrimaryKey personId, QWidget *parent): QWidget(parent) {
    this->personId = personId;
    this->baseModel = DataManager::get().eventsModelForPerson(this, personId);

    treeView = new QTreeView(this);
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    treeView->setUniformRowHeights(true);
    treeView->setRootIsDecorated(true);

    treeView->setModel(baseModel);
    treeView->hideColumn(PersonEventsModel::ROLE_ID);
    treeView->setItemDelegateForColumn(PersonEventsModel::ID,
                                       new FormattedIdentifierDelegate(treeView, FormattedIdentifierDelegate::EVENT));
    treeView->header()->setSortIndicatorClearable(false);
    treeView->header()->setSectionResizeMode(PersonEventsModel::TYPE, QHeaderView::ResizeToContents);
    treeView->header()->setSectionResizeMode(PersonEventsModel::ID, QHeaderView::ResizeToContents);
    treeView->header()->setSectionResizeMode(PersonEventsModel::DATE, QHeaderView::ResizeToContents);
    treeView->expandAll();

    // Wrap in a VBOX for layout reasons.
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(treeView);

    connect(treeView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &EventsOverviewView::handleSelectedNewRow);
    // Support models being reset.
    connect(treeView->model(), &QAbstractItemModel::modelReset, this, [this]() {
        this->handleSelectedNewRow(QItemSelection(), QItemSelection());
    });

    connect(treeView, &QTreeView::doubleClicked, this, &EventsOverviewView::handleDoubleClick);
}

void EventsOverviewView::handleSelectedNewRow(const QItemSelection &selected,
                                              [[maybe_unused]] const QItemSelection &deselected) {
    Q_EMIT this->selectedEvent(this->treeView->selectionModel()->model(), selected);
}

void EventsOverviewView::handleDoubleClick(const QModelIndex &clicked) {
    this->editSelectedEvent();
}

void EventsOverviewView::handleNewEvent() {
    // First, look up the ID for the "default" event role just to be sure.
    auto roleModel = DataManager::get().eventRolesModel();
    auto defaultRole = EventRoles::nameOriginToString[EventRoles::Primary].toString();
    auto defaultEventRoleIndex = roleModel->match(roleModel->index(0, EventRolesModel::ROLE), Qt::DisplayRole,
                                                  defaultRole).first();
    if (!defaultEventRoleIndex.isValid()) {
        qWarning() << "Default role not found, aborting new event.";
        return;
    }
    auto defaultRoleId = roleModel->index(defaultEventRoleIndex.row(), EventRolesModel::ID).data();

    // Also lookup the ID for a first event.
    // TODO: do this intelligently.
    auto typeModel = DataManager::get().eventTypesModel();
    auto defaultType = EventTypes::nameOriginToString[EventTypes::Birth].toString();
    auto defaultEventTypeIndex = typeModel->match(typeModel->index(0, EventTypesModel::TYPE), Qt::DisplayRole,
                                                  defaultType);

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
    auto eventModel = DataManager::get().eventsModel();
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
    auto eventRelationModel = DataManager::get().eventRelationsModel();
    auto eventRelationRecord = eventRelationModel->record();
    eventRelationRecord.setValue(EventRelationsModel::EVENT_ID, newEventId);
    eventRelationRecord.setValue(EventRelationsModel::PERSON_ID, this->personId);
    eventRelationRecord.setValue(EventRelationsModel::ROLE_ID, defaultRoleId);

    if (!eventRelationModel->insertRecord(-1, eventRelationRecord)) {
        QMessageBox::warning(this, tr("Could not event relation"),
                             tr("Problem inserting new event relation into database."));
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

    auto singleEventModel = DataManager::get().singleEventModel(this, newEventId);
    auto singleRelationModel = DataManager::get().singleEventRelationModel(
        this, newEventId, defaultRoleId, this->personId);

    auto editorWindow = new EventEditor(singleRelationModel, singleEventModel, true, this);
    editorWindow->show();
    editorWindow->adjustSize();
}

void EventsOverviewView::editSelectedEvent() {
    // Get the currently selected name.
    const auto selection = this->treeView->selectionModel();
    if (!selection->hasSelection()) {
        return;
    }

    const auto selectRow = selection->selectedRows().first();

    auto eventId = this->treeView->model()->index(selectRow.row(), PersonEventsModel::ID, selectRow.parent()).data();
    auto eventRoleId = this->treeView->model()->index(selectRow.row(), PersonEventsModel::ROLE_ID, selectRow.parent()).
            data();

    auto eventModel = DataManager::get().singleEventModel(this, eventId);
    auto eventRelationModel = DataManager::get().singleEventRelationModel(
        this, eventId, eventRoleId, QVariant::fromValue(personId));

    auto *editorWindow = new EventEditor(eventRelationModel, eventModel, false, this);
    editorWindow->show();
    editorWindow->adjustSize();
}

void EventsOverviewView::removeSelectedEvent() {
    // TODO: for now, we remove the event completely.
    //   However, we also want an unlink button probably?
    //   TODO: add confirmation for this?

    auto selection = this->treeView->selectionModel();
    if (!selection->hasSelection()) {
        qDebug() << "There is no selection, so not deleting anything.";
        return;
    }

    qDebug() << "Selection itself is " << selection;

    auto selectRow = selection->selectedIndexes().first();
    qDebug() << "Selection index is" << selectRow;
    qDebug() << "Selection row is" << selectRow.row();
    qDebug() << "Row count on model is" << treeView->model()->rowCount();
    qDebug() << "Column count on model is " << treeView->model()->columnCount();
    auto eventId = treeView->model()->index(selectRow.row(), PersonEventsModel::ID, selectRow.parent()).data();

    qDebug() << "Will delete event with ID " << eventId.toLongLong();

    // Find the row in the original model.
    auto eventsModel = DataManager::get().eventsModel();
    auto result = eventsModel->match(eventsModel->index(0, EventsModel::ID), Qt::DisplayRole, eventId);
    if (!eventsModel->removeRow(result.first().row())) {
        qWarning() << "Could not delete event" << eventId.toLongLong();
    }
    eventsModel->select();
}
