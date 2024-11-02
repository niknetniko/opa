#include <QHeaderView>
#include <QVBoxLayout>

#include "person_event_overview_view.h"

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
    // Do nothing for now.
}

void EventsOverviewView::editSelectedEvent() {
    // Get the currently selected name.
    const auto selection = this->treeView->selectionModel();
    if (!selection->hasSelection()) {
        return;
    }

    const auto selectRow = selection->selectedRows().first();

    auto eventId = this->treeView->model()->index(selectRow.row(), PersonEventsModel::ID, selectRow.parent()).data();
    auto eventRoleId = this->treeView->model()->index(selectRow.row(), PersonEventsModel::ROLE_ID, selectRow.parent()).data();

    auto eventModel = DataManager::get().singleEventModel(this, eventId);
    auto eventRelationModel = DataManager::get().singleEventRelationModel(this, eventId, eventRoleId, QVariant::fromValue(personId));

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
    qDebug() << "Selection data is" << selectRow.data();
    qDebug() << "Row count on model is" << treeView->model()->rowCount();
    qDebug() << "Column count on model is " << treeView->model()->columnCount();
    auto eventId = treeView->model()->index(selectRow.row(), PersonEventsModel::ID, selectRow.parent()).data();

    qDebug() << "Will delete event with ID " << eventId;

    // Find the row in the original model.
    auto eventsModel = DataManager::get().eventsModel();
    for (int r = 0; r < eventsModel->rowCount(); ++r) {
        auto eventModelId = eventsModel->index(r, EventsModel::ID).data();
        qDebug() << "Considering event with ID " << eventModelId << "for deletion.";
        if (eventModelId == eventId) {
            if (eventsModel->removeRow(r)) {
                qDebug() << "   Deleted the event with ID " << eventId;
            } else {
                qDebug() << "   Found event but could not delete with ID " << eventId;
            }
            eventsModel->select();
            break;
        }
    }
}
