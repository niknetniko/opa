//
// Created by niko on 12/10/24.
//

#ifndef OPA_PERSON_EVENT_OVERVIEW_VIEW_H
#define OPA_PERSON_EVENT_OVERVIEW_VIEW_H

#include <QWidget>
#include <QTreeView>
#include "database/schema.h"

/**
 * Display a list of events for a single person.
 */
class EventsOverviewView : public QWidget {
Q_OBJECT

public:
    explicit EventsOverviewView(IntegerPrimaryKey personId, QWidget *parent);

private:
    QTreeView *treeView;
    QAbstractItemModel *baseModel;
    IntegerPrimaryKey personId;

public Q_SLOTS:

    void handleSelectedNewRow(const QItemSelection &selected, const QItemSelection &deselected);

    void handleDoubleClick(const QModelIndex &clicked);

    /**
     * Initiate adding a new name.
     */
    void handleNewEvent();

    /**
     * Initiate editing the currently selected name.
     */
    void editSelectedEvent();

    /**
     * Remove the currently selected name.
     */
    void removeSelectedEvent();

Q_SIGNALS:

    /**
     * Called when an event is selected by the user.
     */
    void selectedEvent(const QAbstractItemModel *model, const QItemSelection &selected);
};

#endif //OPA_PERSON_EVENT_OVERVIEW_VIEW_H
