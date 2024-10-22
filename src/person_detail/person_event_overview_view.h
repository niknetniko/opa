#pragma once

#include <QTreeView>
#include <QWidget>

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
    void handleSelectedNewRow(const QItemSelection &selected, [[maybe_unused]] const QItemSelection &deselected);

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
