#ifndef OPA_EVENT_TABLE_VIEW_H
#define OPA_EVENT_TABLE_VIEW_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QTableView>

/**
 * This view displays a list of events for one person.
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
class EventTableView: public QWidget {
    Q_OBJECT
public:
    explicit EventTableView(long long personId, QWidget* parent);

    ~EventTableView() override = default;

public Q_SLOTS:
    void handleSelectedNewRow(const QItemSelection &selected, const QItemSelection &deselected);

private:
    QTableView* tableView;
};

#endif //OPA_EVENT_TABLE_VIEW_H
