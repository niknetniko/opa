//
// Created by niko on 2/10/24.
//

#ifndef OPA_NAME_ORIGINS_MANAGEMENT_VIEW_H
#define OPA_NAME_ORIGINS_MANAGEMENT_VIEW_H

#include <QWidget>
#include <QSqlTableModel>

class NameOriginsManagementWindow: public QWidget {
Q_OBJECT

public:
    explicit NameOriginsManagementWindow(QWidget *parent);

public Q_SLOTS:

    void addOrigin();
    void removeOrigin();
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    QSqlTableModel* model;
    QAction* removeAction;

};

#endif //OPA_NAME_ORIGINS_MANAGEMENT_VIEW_H
