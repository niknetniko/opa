#ifndef OPA_PEOPLE_TABLE_VIEW_H
#define OPA_PEOPLE_TABLE_VIEW_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QTableView>

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
class PeopleTableView: public QWidget {
    Q_OBJECT
public:
    explicit PeopleTableView(QWidget* parent);

    ~PeopleTableView() override = default;

public Q_SLOTS:
    void handleSelectedPersonChanged(unsigned long long personId);
    void handleSelectedNewRow(const QItemSelection &selected, const QItemSelection &deselected);

Q_SIGNALS:
#pragma clang diagnostic push
#pragma ide diagnostic ignored "NotImplementedFunctions"
    /**
     * Called when a new person is selected by the table view.
     * @param personId
     */
    void handlePersonSelected(unsigned long long personId);
#pragma clang diagnostic pop

private:
    QTableView* tableView;

    /**
     * Find the row for a person ID.
     * @param personId
     * @return -1 if not found.
     */
    int findRowIndex(unsigned long long personId);
};

#endif //OPA_PEOPLE_TABLE_VIEW_H
