/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "model/distinct_string_model.h"

class NameOriginsModel : public DistinctStringModel {
    Q_OBJECT
public:
    explicit NameOriginsModel(QObject* parent = nullptr);
};
