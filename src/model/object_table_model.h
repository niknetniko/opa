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
    using Setter = std::function<bool(T&, const QVariant&)>;

    explicit ObjectTableModel(QObject* parent = nullptr) : QAbstractTableModel(parent) {
    }

    void addColumn(const QString& header, Extractor extractor) {
        columns.append({header, std::move(extractor), nullptr});
    }

    template<typename FieldType>
    void addColumn(const QString& header, FieldType T::* field) {
        columns.append({header, [field](const T& item) { return QVariant::fromValue(item.*field); }, nullptr});
    }

    void setColumn(int index, const QString& header, Extractor extractor) {
        if (columns.size() <= index) {
            columns.resize(index + 1);
        }
        columns[index] = {header, std::move(extractor), nullptr};
    }

    template <typename FieldType>
    void setColumn(int index, const QString& header, FieldType T::* field) {
        if (columns.size() <= index) {
            columns.resize(index + 1);
        }
        columns[index] = {header, [field](const T& item) { return QVariant::fromValue(item.*field); }, nullptr};
    }

    void setColumn(int index, const QString& header, Extractor extractor, Setter setter) {
        if (columns.size() <= index) {
            columns.resize(index + 1);
        }
        columns[index] = {header, std::move(extractor), std::move(setter)};
    }

    template <typename FieldType>
    void setColumn(int index, const QString& header, FieldType T::* field, Setter setter) {
        if (columns.size() <= index) {
            columns.resize(index + 1);
        }
        columns[index] = {header, [field](const T& item) { return QVariant::fromValue(item.*field); }, std::move(setter)};
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

    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override {
        if (!index.isValid() || index.row() >= items.size() || index.column() >= columns.size()) {
            return false;
        }
        if (role == Qt::EditRole) {
            auto& setter = columns[index.column()].setter;
            if (setter) {
                T& item = items[index.row()];
                if (setter(item, value)) {
                    Q_EMIT dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
                    return true;
                }
            }
        }
        return false;
    }

    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override {
        Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);
        if (index.isValid() && index.column() < columns.size() && columns[index.column()].setter != nullptr) {
            return defaultFlags | Qt::ItemIsEditable;
        }
        return defaultFlags;
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
        Setter setter;
    };

    QList<T> items;
    QList<ColumnDef> columns;
};
