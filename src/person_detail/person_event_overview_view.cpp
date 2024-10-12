//
// Created by niko on 12/10/24.
//

#include <QHeaderView>
#include <QVBoxLayout>

#include "person_event_overview_view.h"
#include "data/data_manager.h"
#include "utils/formatted_identifier_delegate.h"
#include "data/event.h"

EventsOverviewView::EventsOverviewView(IntegerPrimaryKey personId, QWidget *parent) {
    this->personId = personId;
    this->baseModel = DataManager::getInstance(this)->eventsModelForPerson(this, personId);

    treeView = new QTreeView(this);
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    treeView->setUniformRowHeights(true);
    treeView->setRootIsDecorated(false);
    treeView->setSortingEnabled(true);
    // Sort on default column.
    treeView->sortByColumn(1, Qt::AscendingOrder);
    // We are done setting up, attach the model.
    treeView->setModel(baseModel);
    treeView->setItemDelegateForColumn(PersonEventsModel::ID,
                                       new FormattedIdentifierDelegate(FormattedIdentifierDelegate::EVENT));
    treeView->header()->setSortIndicatorClearable(false);

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

void EventsOverviewView::handleSelectedNewRow(const QItemSelection &selected, const QItemSelection &deselected) {
    Q_EMIT this->selectedEvent(this->treeView->selectionModel()->model(), selected);
}

void EventsOverviewView::handleDoubleClick(const QModelIndex &clicked) {
    // Do nothing for now.
}

void EventsOverviewView::handleNewEvent() {
    // Do nothing for now.
}

void EventsOverviewView::editSelectedEvent() {

}

void EventsOverviewView::removeSelectedEvent() {

}

