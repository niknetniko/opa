/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "choose_existing_reference_window.h"

class ChooseExistingPersonWindow : public ChooseExistingReferenceWindow {
    Q_OBJECT

public:
    static QVariant selectPerson(QWidget* parent);

protected:
    explicit ChooseExistingPersonWindow(QWidget* parent);
};
