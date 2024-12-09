/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "choose_existing_reference_window.h"

class QComboBox;

struct ExistingEventSelection {
    QVariant eventId;
    QVariant roleId;

    [[nodiscard]] bool isValid() const;
};

QDebug operator<<(QDebug dbg, const ExistingEventSelection& selection);

class ChooseExistingEventWindow : public ChooseExistingReferenceWindow {
    Q_OBJECT

public:
    static ExistingEventSelection selectEventAndRole(QWidget* parent);

public Q_SLOTS:
    void accept() override;

protected:
    explicit ChooseExistingEventWindow(QWidget* parent);

private:
    QVariant selectedRole;
    QComboBox* eventRoleComboBox = nullptr;
};
