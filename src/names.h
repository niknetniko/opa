//
// Created by niko on 8/02/24.
//

#ifndef OPA_NAMES_H
#define OPA_NAMES_H

#include <QString>
#include <QAbstractItemModel>
#include <QSqlRecord>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QSqlTableModel>
#include <KRearrangeColumnsProxyModel>


namespace Names {
    Q_NAMESPACE

    enum Origin {
        NONE, // No origin was given.
        UNKNOWN, // Origin is not known.
        PATRILINEAL, // Inherited from father.
        MATRILINEAL, // Inherited from mother.
        GIVEN, // Given to the person by someone else.
        CHOSEN, // Chosen by the person himself.
        PATRONYMIC, // Derived from father's given name(s).
        MATRONYMIC, // Derived from mother's given name(s).
        OCCUPATION, // Derived from person's occupation.
        LOCATION, // Derived from person's location
    };

    Q_ENUM_NS(Origin)

    QString origin_to_display(const Origin &origin);


    QString construct_display_name(const QString &titles, const QString &givenNames, const QString &prefix,
                                   const QString &surname);
};

/**
 * The data model for the PeopleTableView.
 */
class NamesTableModel : public QSqlTableModel {
Q_OBJECT

public:
    explicit NamesTableModel(long long personId, QObject *parent = nullptr);

    [[nodiscard]] QVariant data(const QModelIndex &item, int role) const override;

private:
    long long personId;
    QVector<int> editable;

    void regenerateQuery();
};

class SelectedDataNamesTableModel : public KRearrangeColumnsProxyModel {
Q_OBJECT

public:
    explicit SelectedDataNamesTableModel(long long personId, QObject *parent = nullptr);

};

class SortableAndFilterableModel : public QSortFilterProxyModel {
Q_OBJECT

public:
    explicit SortableAndFilterableModel(long long id, QObject *parent = nullptr);

    long long personId;
};

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
class NamesTableView : public QWidget {
Q_OBJECT
public:
    explicit NamesTableView(long long personId, QWidget *parent);

    ~NamesTableView() override = default;

public Q_SLOTS:

    void handleSelectedNewRow(const QItemSelection &selected, const QItemSelection &deselected);

private:
    QTableView *tableView;
    SortableAndFilterableModel *model;
};


#endif //OPA_NAMES_H
