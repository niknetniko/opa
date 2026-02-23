/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QAbstractTableModel>
#include <QVariant>
#include <functional>

template<typename T>
class ObjectTableModel : public QAbstractTableModel {
public:
    using Extractor = std::function<QVariant(const T&)>;

    explicit ObjectTableModel(QObject* parent = nullptr) : QAbstractTableModel(parent) {
    }

    void addColumn(const QString& header, Extractor extractor) {
        columns.append({header, std::move(extractor)});
    }

    template<typename FieldType>
    void addColumn(const QString& header, FieldType T::* field) {
        columns.append({header, [field](const T& item) { return QVariant::fromValue(item.*field); }});
    }

    void setColumn(int index, const QString& header, Extractor extractor) {
        if (columns.size() <= index) {
            columns.resize(index + 1);
        }
        columns[index] = {header, std::move(extractor)};
    }

    template <typename FieldType>
    void setColumn(int index, const QString& header, FieldType T::* field) {
        if (columns.size() <= index) {
            columns.resize(index + 1);
        }
        columns[index] = {header, [field](const T& item) { return QVariant::fromValue(item.*field); }};
    }

    void setItems(const QList<T>& itemsParam) {
        beginResetModel();
        items = itemsParam;
        endResetModel();
    }

    [[nodiscard]] const QList<T>& getItems() const {
        return items;
    }

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        if (parent.isValid()) {
            return 0;
        }
        return items.size();
    }

    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override {
        if (parent.isValid()) {
            return 0;
        }
        return columns.size();
    }

    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= items.size() || index.column() >= columns.size()) {
            return {};
        }

        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            const T& item = items[index.row()];
            return columns[index.column()].extractor(item);
        }

        return {};
    }

    [[nodiscard]] QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (role != Qt::DisplayRole || orientation != Qt::Horizontal || section >= columns.size()) {
            return {};
        }

        return columns[section].header;
    }

private:
    struct ColumnDef {
        QString header;
        Extractor extractor;
    };

    QList<T> items;
    QList<ColumnDef> columns;
};
