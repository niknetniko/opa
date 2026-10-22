/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QModelIndex>
#include <QString>
#include <QVariant>
#include <functional>
#include <optional>


/**
 * Base class for shared logic between Qt models that work on a "dataclass", meaning T.
 *
 * @tparam T The dataclass type this model works on.
 * @tparam QtModel The actual model type this class is derived from.
 */
template<typename T, typename QtModel>
class BaseObjectModel : public QtModel {
public:
    using Extractor = std::function<QVariant(const T&)>;
    using Setter = std::function<bool(T&, const QVariant&)>;

    struct ColumnDef {
        QString header;
        Extractor extractor;
        Setter setter;
    };

    explicit BaseObjectModel(QObject* parent = nullptr) : QtModel(parent) {
    }

    void addColumn(const QString& header, Extractor extractor) {
        Q_ASSERT(items.isEmpty());
        columns.append({header, std::move(extractor), nullptr});
    }

    template<typename FieldType>
    void addColumn(const QString& header, FieldType T::* field) {
        this->addColumn(header, [field](const T& item) { return toVariant(item.*field); });
    }

    void setColumn(int index, const QString& header, Extractor extractor) {
        this->setColumn(index, header, std::move(extractor), nullptr);
    }

    template<typename FieldType>
    void setColumn(int index, const QString& header, FieldType T::* field) {
        this->setColumn(index, header, field, nullptr);
    }

    void setColumn(int index, const QString& header, Extractor extractor, Setter setter) {
        this->putColumn(index, {header, std::move(extractor), std::move(setter)});
    }

    template<typename FieldType>
    void setColumn(int index, const QString& header, FieldType T::* field, Setter setter) {
        this->setColumn(index, header, [field](const T& item) { return toVariant(item.*field); }, std::move(setter));
    }

    [[nodiscard]] const QList<T>& getItems() const {
        return items;
    }

    void setItems(const QList<T>& itemsParam) {
        this->beginResetModel();
        items = itemsParam;
        this->rebuild();
        this->endResetModel();
    }

    [[nodiscard]] QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (role != Qt::DisplayRole || orientation != Qt::Horizontal || section < 0 || section >= columns.size()) {
            return {};
        }

        return columns[section].header;
    }

    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
        if (role != Qt::DisplayRole && role != Qt::EditRole) {
            return {};
        }

        const auto itemIndex = this->itemIndex(index);
        if (!itemIndex || index.column() >= columns.size() || !columns[index.column()].extractor) {
            return {};
        }

        return columns[index.column()].extractor(items[*itemIndex]);
    }

    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override {
        if (role != Qt::EditRole) {
            return false;
        }

        const auto itemIndex = this->itemIndex(index);
        if (!itemIndex) {
            return false;
        }

        const auto& setter = columns[index.column()].setter;
        if (!setter) {
            return false;
        }

        if (T& item = items[*itemIndex]; setter(item, value)) {
            Q_EMIT this->dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
            return true;
        }

        return false;
    }


    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override {
        Qt::ItemFlags defaultFlags = QtModel::flags(index);

        if (const auto itemIndex = this->itemIndex(index);
            itemIndex && index.column() < columns.size() && columns[index.column()].setter != nullptr) {
            return defaultFlags | Qt::ItemIsEditable;
        }

        return defaultFlags;
    }

    [[nodiscard]] const T* itemAt(const QModelIndex& index) const {
        if (!index.isValid()) {
            return nullptr;
        }

        const auto itemIndex = this->itemIndex(index);
        if (!itemIndex) {
            return nullptr;
        }

        return &items[*itemIndex];
    }

protected:
    [[nodiscard]] virtual std::optional<qsizetype> itemIndex(const QModelIndex& index) const = 0;

    virtual void rebuild() {
    }

    QList<T> items;
    QList<ColumnDef> columns;

private:
    void putColumn(int index, ColumnDef def) {
        Q_ASSERT(items.isEmpty());

        if (columns.size() <= index) {
            columns.resize(index + 1);
        }

        columns[index] = std::move(def);
    }

    template<typename V>
    static QVariant toVariant(const V& value) {
        // std::optional
        if constexpr (requires {
                          value.has_value();
                          *value;
                      }) {
            return value ? QVariant::fromValue(*value) : QVariant {};
        } else {
            return QVariant::fromValue(value);
        }
    }
};
