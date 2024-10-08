//
// Created by niko on 8/02/24.
//

#ifndef OPA_NAMES_OVERVIEW_VIEW_H
#define OPA_NAMES_OVERVIEW_VIEW_H

#include <QString>
#include <QAbstractItemModel>
#include <QSqlRecord>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QSqlTableModel>
#include <QTreeView>
#include <KRearrangeColumnsProxyModel>
#include <QSqlRelationalTableModel>
#include "database/schema.h"


namespace Names {
    Q_NAMESPACE

    QString construct_display_name(const QString &titles, const QString &givenNames, const QString &prefix,
                                   const QString &surname);
}

/**
 * This view displays a list of names for one person.
 */
class NamesOverviewView : public QWidget {
Q_OBJECT
public:
    explicit NamesOverviewView(IntegerPrimaryKey personId, QWidget *parent);

private:
    QTreeView *treeView;
    QAbstractItemModel *baseModel;
    IntegerPrimaryKey personId;

    void moveSelectedNameToPosition(int from, int to);

public Q_SLOTS:
    void handleSelectedNewRow(const QItemSelection &selected, const QItemSelection &deselected);
    void handleDoubleClick(const QModelIndex& clicked);

    /**
     * Initiate adding a new name.
     */
    void handleNewName();
    /**
     * Initiate editing the currently selected name.
     */
    void editSelectedName();
    /**
     * Remove the currently selected name.
     */
    void removeSelectedName();
    /**
     * Move the selected name down one row.
     */
    void moveSelectedNameDown();
    /**
     * Move the selected name up one row.
     */
    void moveSelectedNameUp();

Q_SIGNALS:
    /**
     * Called when a person is selected by the user.
     */
    void selectedName(const QAbstractItemModel *model, const QItemSelection &selected);

    /**
     * Called when the sorting in the UI changes.
     */
    void sortChanged(const QAbstractItemModel *model, const QItemSelection &selected, int sortIndex);
};

#endif //OPA_NAMES_OVERVIEW_VIEW_H