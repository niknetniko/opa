//
// Created by niko on 2/10/24.
//

#ifndef OPA_EVENT_ROLES_MANAGEMENT_VIEW_H
#define OPA_EVENT_ROLES_MANAGEMENT_VIEW_H

#include "simple_list_manager.h"

class EventRolesManagementWindow : public SimpleListManagementWindow {
    Q_OBJECT

public:
    explicit EventRolesManagementWindow(QWidget *parent);

public Q_SLOTS:
    bool repairConfirmation() override;


    void removeMarkedReferences(const QHash<QString, QVector<IntegerPrimaryKey> > &valueToIds,
                                const QHash<IntegerPrimaryKey, QString> &idToValue) override;

    bool isUsed(const QVariant &id) override;
};

#endif //OPA_EVENT_ROLES_MANAGEMENT_VIEW_H
