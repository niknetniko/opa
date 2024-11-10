/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "simple_list_manager.h"

class NameOriginsManagementWindow : public SimpleListManagementWindow {
    Q_OBJECT

public:
    explicit NameOriginsManagementWindow();

protected:
    bool repairConfirmation() override;

    void removeMarkedReferences(
        const QHash<QString, QVector<IntegerPrimaryKey>>& valueToIds, const QHash<IntegerPrimaryKey, QString>& idToValue
    ) override;

    bool isUsed(const QVariant& id) override;

    [[nodiscard]] QString translatedItemCount(int itemCount) const override;

    [[nodiscard]] QString translatedItemDescription(const QString& item, bool isBuiltIn) const override;
};
