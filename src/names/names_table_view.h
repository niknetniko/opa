//
// Created by niko on 8/02/24.
//

#ifndef OPA_NAMES_TABLE_VIEW_H
#define OPA_NAMES_TABLE_VIEW_H

#include <QString>
#include <QAbstractItemModel>
#include <QSqlRecord>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QSqlTableModel>
#include <KRearrangeColumnsProxyModel>
#include <QSqlRelationalTableModel>
#include "database/schema.h"


namespace Names {
    Q_NAMESPACE

    QString construct_display_name(const QString &titles, const QString &givenNames, const QString &prefix,
                                   const QString &surname);
}

class NameOriginTableModel: public QSqlTableModel {
    Q_OBJECT

public:
    static const int ID = 0;
    static const int ORIGIN = 1;

    explicit NameOriginTableModel(QObject* parent = nullptr): QSqlTableModel(parent) {
        this->setTable(Schema::NameOrigins::TableName);
    };
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
    QAbstractItemModel *baseModel;
    IntegerPrimaryKey personId;

public Q_SLOTS:

    void handleNewName();

    void handleSelectedNewRow(const QItemSelection &selected, const QItemSelection &deselected);
    void handleDoubleClick(const QModelIndex& clicked);

    void editSelectedName();
    void removeSelectedName();

Q_SIGNALS:
    /**
     * Called when a person is selected by the user.
     *
     * @param selected
     * @param deselected
     */
    void selectedName(const QAbstractItemModel &model, const QItemSelection &selected);
};

#endif //OPA_NAMES_TABLE_VIEW_H
