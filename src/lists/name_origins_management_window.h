#pragma once

#include "simple_list_manager.h"

class NameOriginsManagementWindow : public SimpleListManagementWindow {
    Q_OBJECT

public:
    explicit NameOriginsManagementWindow(QWidget *parent);

protected:
    bool repairConfirmation() override;

    void removeMarkedReferences(const QHash<QString, QVector<IntegerPrimaryKey> > &valueToIds,
                                const QHash<IntegerPrimaryKey, QString> &idToValue) override;

    bool isUsed(const QVariant &id) override;
};
