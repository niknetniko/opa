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
#include "database/schema.h"


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
}

/**
 * Model for the "names" SQL table.
 *
 * The model contains all names in the database.
 *
 * The table will automatically enforce constraints, e.g. that only one row may
 * be the main name at a time.
 *
 * The columns and their positions is exposed as special variables. Use those.
 */
class NamesTableModel : public QSqlTableModel {
Q_OBJECT

public:
    static const int ID = 0;
    static const int PERSON_ID = 1;
    static const int MAIN = 2;
    static const int TITLES = 3;
    static const int GIVEN_NAMES = 4;
    static const int PREFIX = 5;
    static const int SURNAME = 6;
    static const int ORIGIN = 7;

    explicit NamesTableModel(QObject *parent = nullptr) : NamesTableModel(-1, parent) {};

    explicit NamesTableModel(IntegerPrimaryKey personId, QObject *parent = nullptr);

    [[nodiscard]] QVariant data(const QModelIndex &item, int role) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

private:
    // -1 if full table, otherwise the ID of the person whose names to use.
    IntegerPrimaryKey personId;
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
    explicit NamesTableView(IntegerPrimaryKey personId, QWidget *parent);

private:
    QTableView *tableView;
    NamesTableModel *baseModel;
    IntegerPrimaryKey personId;

public Q_SLOTS:

    void handleNewName();

    void handleSelectedNewRow(const QItemSelection &selected, const QItemSelection &deselected);
};

#endif //OPA_NAMES_H
