//
// Created by niko on 28/09/24.
//

#ifndef OPA_PERSON_NAME_TAB_H
#define OPA_PERSON_NAME_TAB_H

#include <QWidget>
#include <QTableView>
#include "database/schema.h"

class PersonNameTab: public QWidget {
    Q_OBJECT

public:
    explicit PersonNameTab(IntegerPrimaryKey person, QWidget* parent);

public Q_SLOTS:
    void onNameSelected(const QAbstractItemModel *model, const QItemSelection &selected) const;
    void onSortChanged(const QAbstractItemModel *model, const QItemSelection &selected, int logicalIndex) const;

private:
    QAction* addAction;
    QAction* removeAction;
    QAction* editAction;
    QAction* upAction;
    QAction* downAction;
};

#endif //OPA_PERSON_NAME_TAB_H
