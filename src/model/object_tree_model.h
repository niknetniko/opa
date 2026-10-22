/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "database/schema.h"
#include "model/object_table_model.h"

#include <QVariant>
#include <functional>
#include <ranges>
#include <variant>


struct TreeNode {
    /**
     * Index of the parent in the tree, or -1 if top-level node.
     */
    qsizetype parentIndex;
    /**
     * Indices of the children in the tree.
     */
    QList<qsizetype> children;
    /**
     * The position in the parent's children array (or in the top level).
     */
    qsizetype rowPosition;
    /**
     * Either:
     * - The index of the item in `items`
     * - The virtual group
     */
    std::variant<qsizetype, QVariant> content;
};


template<typename T>
class ObjectTreeModel : public BaseObjectModel<T, QAbstractItemModel> {

public:
    using Grouper = std::function<QVariant(const T&)>;
    using ParentLink = std::function<IntegerPrimaryKey(const T&)>;

    explicit ObjectTreeModel(const std::variant<Grouper, ParentLink> parentProvider, QObject* parent = nullptr) :
        BaseObjectModel<T, QAbstractItemModel>(parent),
        parentProvider(parentProvider) {
    }

    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override {
        if (!this->hasIndex(row, column, parent)) {
            return {};
        }

        if (!parent.isValid()) {
            return this->createIndex(row, column, this->topLevel[row]);
        }

        return this->createIndex(row, column, this->tree[parent.internalId()].children[row]);
    }

    [[nodiscard]] QModelIndex parent(const QModelIndex& child) const override {
        if (!child.isValid()) {
            return {};
        }

        const auto parentIndex = this->tree[child.internalId()].parentIndex;
        if (parentIndex < 0) {
            return {};
        }

        const auto parentPosition = this->tree[parentIndex].rowPosition;

        return this->createIndex(parentPosition, 0, parentIndex);
    }

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        if (!parent.isValid()) {
            return this->topLevel.size();
        }

        if (parent.column() > 0) {
            // Only column 0 has children.
            return 0;
        }

        return this->tree[parent.internalId()].children.size();
    }

    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return this->columns.size();
    }

    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
        if (!index.isValid()) {
            return {};
        }

        const auto& node = this->tree[index.internalId()];
        if (const auto* group = std::get_if<QVariant>(&node.content)) {
            if (role == Qt::DisplayRole && index.column() == 0) {
                return *group;
            }

            return {};
        }

        return BaseObjectModel<T, QAbstractItemModel>::data(index, role);
    }

protected:
    void rebuild() override {
        // Yeet the existing tree.
        this->tree.clear();
        this->topLevel.clear();

        QList<QVariant> virtualGroups;

        if (std::holds_alternative<Grouper>(this->parentProvider)) {
            const auto& getter = std::get<Grouper>(this->parentProvider);
            for (const auto [itemIndex, item]: std::views::enumerate(std::as_const(this->items))) {
                auto group = getter(item);

                qsizetype parentIndex = -1;

                if (group.isValid()) {
                    auto existing = virtualGroups.indexOf(group);
                    if (existing == -1) {
                        virtualGroups.push_back(group);
                        parentIndex = this->items.size() + virtualGroups.size() - 1;
                    } else {
                        parentIndex = this->items.size() + existing;
                    }
                }

                this->tree.push_back({
                    .parentIndex = parentIndex,
                    .children = {},
                    .rowPosition = -1,
                    .content = itemIndex,
                });
            }
        } else {
            Q_ASSERT(std::holds_alternative<ParentLink>(this->parentProvider));
            const auto& parentLinker = std::get<ParentLink>(this->parentProvider);

            QHash<IntegerPrimaryKey, qsizetype> itemIdToPosition;
            itemIdToPosition.reserve(this->items.size());
            for (const auto [itemIndex, item]: std::views::enumerate(std::as_const(this->items))) {
                itemIdToPosition[item.id] = itemIndex;
            }

            for (const auto [itemIndex, item]: std::views::enumerate(std::as_const(this->items))) {
                auto parentId = parentLinker(item);

                qsizetype parentIndex = -1;
                if (parentId == item.id) {
                    // We do not check further than one level, the caller is responsible for preventing loops.
                    throw std::runtime_error("Parent link cannot be itself");
                }
                if (parentId != -1) {
                    parentIndex = itemIdToPosition.value(parentId, -1);
                }

                this->tree.push_back({
                    .parentIndex = parentIndex,
                    .children = {},
                    .rowPosition = -1,
                    .content = itemIndex,
                });
            }
        }

        // Add the virtual groups.
        for (const auto& groupToAdd: virtualGroups) {
            this->tree.push_back({
                .parentIndex = -1,
                .children = {},
                .rowPosition = -1,
                .content = groupToAdd,
            });
        }

        // Add the children.
        for (auto [i, node]: std::views::enumerate(this->tree)) {
            if (node.parentIndex != -1) {
                auto& parentChildren = this->tree[node.parentIndex].children;
                node.rowPosition = parentChildren.size();
                parentChildren.push_back(i);
            } else {
                node.rowPosition = this->topLevel.size();
                this->topLevel.push_back(i);
            }
        }
    }

    [[nodiscard]] std::optional<qsizetype> itemIndex(const QModelIndex& index) const override {
        if (!index.isValid()) {
            return std::nullopt;
        }

        const auto& node = this->tree[index.internalId()];
        if (const auto* result = std::get_if<qsizetype>(&node.content)) {
            return *result;
        }

        return std::nullopt;
    }

private:
    std::variant<Grouper, ParentLink> parentProvider;

    QList<TreeNode> tree;
    QList<qsizetype> topLevel;
};
