/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "simple_list_manager.h"

#include <QAction>

class LocationTypesManagementWindow : public SimpleListManagementWindow {
    Q_OBJECT

public:
    explicit LocationTypesManagementWindow();

public Q_SLOTS:
    bool repairConfirmation() override;

    void repairItems() override;

    void removeMarkedReferences(
        const QHash<QString, QVector<IntegerPrimaryKey>>& valueToIds, const QHash<IntegerPrimaryKey, QString>& idToValue
    ) override;

    bool isUsed(const QVariant& id) override;

protected:
    [[nodiscard]] QVariant doAddItem() const override;

    bool doRemoveItem(const QVariant& id) const override;

    [[nodiscard]] QString translatedItemCount(int itemCount) const override;

    [[nodiscard]] QString translatedItemDescription(const QString& item, bool isBuiltIn) const override;

private:
    QAction* translationsAction = nullptr;
};
