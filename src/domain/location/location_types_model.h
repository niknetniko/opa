/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#pragma once

#include "model/distinct_string_model.h"

class LocationTypesModel : public DistinctStringModel {
    Q_OBJECT
public:
    explicit LocationTypesModel(QObject* parent = nullptr);
};
