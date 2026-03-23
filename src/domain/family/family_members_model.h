/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "family_entities.h"

#include <QAbstractItemModel>
#include <QMap>

/**
 * A tree model that renders partners and children of a person.
 *
 * Top-level rows are partner rows (marriage/relationship events) or a pseudo-row for
 * children without a known co-parent. Each top-level row has children as sub-rows.
 *
 * Column layout matches FamilyDisplayModel: TYPE, DATE, PERSON_ID, DISPLAY_NAME, EVENT_ID.
 */
class FamilyMembersModel : public QAbstractItemModel {
    Q_OBJECT

public:
    enum Columns { TYPE = 0, DATE, PERSON_ID, DISPLAY_NAME, EVENT_ID };
    Q_ENUM(Columns);

    explicit FamilyMembersModel(IntegerPrimaryKey personId, QObject* parent = nullptr);

    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QModelIndex parent(const QModelIndex& child) const override;
    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;

    [[nodiscard]] bool hasBastardChildren() const;

public Q_SLOTS:
    void reload();

private:
    IntegerPrimaryKey personId;
    QList<FamilyMemberEntity> items;

    // Maps top-level row index -> list of child row indices in `items`
    QMap<int, QList<int>> mapping;
    // Sentinel for children with no known co-parent
    static constexpr int BASTARD_PARENT_KEY = std::numeric_limits<int>::max();

    void rebuildMapping();
    [[nodiscard]] QVariant dataForItem(const FamilyMemberEntity& item, int column) const;
    [[nodiscard]] bool isBastardParentRow(const QModelIndex& index) const;
};
