/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#pragma once

#include "model/distinct_string_model.h"

class SourceTypesModel : public DistinctStringModel {
    Q_OBJECT
public:
    explicit SourceTypesModel(QObject *parent = nullptr);
};
