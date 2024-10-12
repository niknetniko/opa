//
// Created by niko on 28/09/24.
//

#ifndef OPA_PERSON_EVENT_TAB_H
#define OPA_PERSON_EVENT_TAB_H

#include <QWidget>
#include <QTableView>
#include "database/schema.h"

class PersonEventTab: public QWidget {
    Q_OBJECT

public:
    explicit PersonEventTab(IntegerPrimaryKey person, QWidget* parent);

public Q_SLOTS:
    void onEventSelected(const QAbstractItemModel *model, const QItemSelection &selected);

private:
    QAction* addAction;
    QAction* removeAction;
    QAction* editAction;
};

#endif //OPA_PERSON_EVENT_TAB_H
