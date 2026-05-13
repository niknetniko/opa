/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "family_entities.h"

#include <QAbstractItemModel>
#include <QHash>
#include <QList>

/**
 * Two-level tree model for all families.
 *
 * Top-level rows represent families (named after parent surnames).
 * Child rows represent individual event × participant combinations within that family.
 *
 * Column layout: DISPLAY_NAME, TYPE, DATE, ROLE, PERSON_ID, EVENT_ID, FAMILY_ID.
 */
class FamilyListModel : public QAbstractItemModel {
    Q_OBJECT

public:
    enum Columns { DISPLAY_NAME = 0, TYPE, DATE, ROLE, PERSON_ID, EVENT_ID, FAMILY_ID };
    Q_ENUM(Columns);

    explicit FamilyListModel(QObject* parent = nullptr);

    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QModelIndex parent(const QModelIndex& child) const override;
    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;

public Q_SLOTS:
    void reload();

private:
    QList<FamilyOverviewRow> rows;
    QList<IntegerPrimaryKey> families;
    QHash<IntegerPrimaryKey, QList<int>> childRows;
    QHash<IntegerPrimaryKey, QString> familyDisplayNames;

    void rebuildMapping();
};
