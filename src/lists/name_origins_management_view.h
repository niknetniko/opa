//
// Created by niko on 2/10/24.
//

#ifndef OPA_NAME_ORIGINS_MANAGEMENT_VIEW_H
#define OPA_NAME_ORIGINS_MANAGEMENT_VIEW_H

#include <QWidget>
#include <QSqlTableModel>
#include <QTableView>

#include "simple_list_manager.h"

class NameOriginsManagementWindow : public SimpleListManagementWindow {
    Q_OBJECT

public:
    explicit NameOriginsManagementWindow(QWidget *parent);

protected:
    QString addItemDescription() override;
    bool repairConfirmation() override;
    QString removeItemDescription() override;
    bool removeMarkedReferences(const QHash<QString, QVector<IntegerPrimaryKey>> &valueToIds, const QHash<IntegerPrimaryKey, QString> &idToValue) override;
    QString repairItemDescription() override;
    bool isUsed(const QVariant &id) override;
};

#endif //OPA_NAME_ORIGINS_MANAGEMENT_VIEW_H
