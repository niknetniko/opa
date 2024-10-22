//
// Created by niko on 22/10/24.
//

#ifndef EVENT_TYPES_MANAGEMENT_WINDOW_H
#define EVENT_TYPES_MANAGEMENT_WINDOW_H
#include "simple_list_manager.h"

class EventTypesManagementWindow : public SimpleListManagementWindow {
    Q_OBJECT

public:
    explicit EventTypesManagementWindow(QWidget *parent);

    public Q_SLOTS:
        bool repairConfirmation() override;


    void removeMarkedReferences(const QHash<QString, QVector<IntegerPrimaryKey> > &valueToIds,
                                const QHash<IntegerPrimaryKey, QString> &idToValue) override;

    bool isUsed(const QVariant &id) override;
};

#endif //EVENT_TYPES_MANAGEMENT_WINDOW_H
