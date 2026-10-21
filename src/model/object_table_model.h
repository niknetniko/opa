/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "base_object_model.h"

#include <QAbstractTableModel>

template<typename T>
class ObjectTableModel : public BaseObjectModel<T, QAbstractTableModel> {
public:
    explicit ObjectTableModel(QObject* parent = nullptr) : BaseObjectModel<T, QAbstractTableModel>(parent) {
    }

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        if (parent.isValid()) {
            return 0;
        }
        return this->items.size();
    }

    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override {
        if (parent.isValid()) {
            return 0;
        }
        return this->columns.size();
    }

protected:
    std::optional<qsizetype> itemIndex(const QModelIndex& index) const override {
        if (!index.isValid() || index.row() >= this->items.size()) {
            return std::nullopt;
        }

        return index.row();
    }
};
