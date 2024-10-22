#pragma once

#include <QTableView>
#include <QWidget>

#include "database/schema.h"

/**
 * This view displays a list of people.
 *
 * The intent of the view is to be configurable, with features such as:
 *
 * - Optional searching
 * - Configurable columns
 *
 * However, currently, the view is not configurable at all.
 *
 * When a person is selected, a signal will be emitted. However,
 * the act of selecting a person will not cause the globally selected person
 * to change. This is the job of a listener.
 */
class PeopleOverviewView : public QWidget {
    Q_OBJECT

public:
    explicit PeopleOverviewView(QWidget *parent);

    ~PeopleOverviewView() override = default;

public Q_SLOTS:
    void handleSelectedNewRow(const QItemSelection &selected);

Q_SIGNALS:
    /**
     * Called when a new person is selected by the table view.
     */
    void handlePersonSelected(IntegerPrimaryKey personId);

private:
    QTableView *tableView;
};
