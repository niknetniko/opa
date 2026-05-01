/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "choose_existing_reference_window.h"

class ChooseExistingMediaWindow : public ChooseExistingReferenceWindow {
    Q_OBJECT

public:
    static QVariant selectMedia(QWidget* parent);

protected:
    explicit ChooseExistingMediaWindow(QWidget* parent);
};
