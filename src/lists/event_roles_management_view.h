//
// Created by niko on 2/10/24.
//

#ifndef OPA_EVENT_ROLES_MANAGEMENT_VIEW_H
#define OPA_EVENT_ROLES_MANAGEMENT_VIEW_H

#include <QWidget>
#include <QSqlTableModel>
#include <QTableView>

class EventRolesManagementWindow : public QWidget {
    Q_OBJECT

public:
    explicit EventRolesManagementWindow(QWidget *parent);

public Q_SLOTS:
    void addRole();

    void removeRole();

    void repairRoles();

    void onSelectionChanged(const QItemSelection &selected, [[maybe_unused]] const QItemSelection &deselected);

private:
    QSqlTableModel *model;
    QAction *removeAction;
    QTableView *tableView;
};

#endif //OPA_EVENT_ROLES_MANAGEMENT_VIEW_H
