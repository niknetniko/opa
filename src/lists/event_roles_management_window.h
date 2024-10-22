#pragma once

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
