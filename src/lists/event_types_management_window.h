#pragma once

#include "simple_list_manager.h"

class EventTypesManagementWindow : public SimpleListManagementWindow {
    Q_OBJECT

public:
    explicit EventTypesManagementWindow();

public Q_SLOTS:
    bool repairConfirmation() override;


    void removeMarkedReferences(const QHash<QString, QVector<IntegerPrimaryKey> > &valueToIds,
                                const QHash<IntegerPrimaryKey, QString> &idToValue) override;

    bool isUsed(const QVariant &id) override;

    QString translatedItemCount(int itemCount) const override;

    QString translatedItemDescription(const QString &item, bool isBuiltIn) const override;
};
