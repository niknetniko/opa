#pragma once

#include "database/schema.h"

#include <QTableView>
#include <QWidget>

class PersonNameTab : public QWidget {
    Q_OBJECT

public:
    explicit PersonNameTab(IntegerPrimaryKey person, QWidget* parent);

public Q_SLOTS:
    void onNameSelected(const QAbstractItemModel* model, const QItemSelection& selected) const;

    void onSortChanged(const QAbstractItemModel* model, const QItemSelection& selected, int logicalIndex) const;

private:
    QAction* addAction;
    QAction* removeAction;
    QAction* editAction;
    QAction* upAction;
    QAction* downAction;
};
