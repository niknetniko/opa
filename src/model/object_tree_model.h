/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "model/object_table_model.h"

#include <QAbstractTableModel>
#include <QVariant>
#include <functional>


template<typename T>
class ObjectTreeModel : public ObjectTableModel<T> {

public:
    using Grouper = std::function<QVariant(const T&)>;

    explicit ObjectTreeModel(const Grouper grouper, QObject* parent = nullptr) :
        ObjectTableModel<T>(parent),
        grouper(grouper) {
    }


private:
    Grouper grouper;
};
