/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "event.h"
#include <database/schema.h>

#include <QAbstractProxyModel>


// TODO, we need:
//  - list of partners
//  - list of children
// Then, the partners become the first level of the model
// The children become the second model.
// If we want this in one big model, we need the following:

// SELECT * FROM events WHERE event_type = (relationships || parents)


/**
 * Renders all spouses and children of a single person into a tree view of families.
 */
class FamilyProxyModel : public QAbstractProxyModel {

    Q_OBJECT

public:
    static constexpr int EVENT_ID = 0;
    static constexpr int ROLE_ID = 1;
    static constexpr int ROLE = 2;
    static constexpr int PERSON_ID = 3;
    static constexpr int TYPE_ID = 4;
    static constexpr int TYPE = 5;
    static constexpr int DATE = 6;
    static constexpr int GIVEN_NAMES = 7;
    static constexpr int PREFIX = 8;
    static constexpr int SURNAME = 9;

    FamilyProxyModel(IntegerPrimaryKey person, QObject* parent);

    [[nodiscard]] QModelIndex parent(const QModelIndex& child) const override;
    bool hasChildren(const QModelIndex& parent) const override;

    [[nodiscard]] QModelIndex mapFromSource(const QModelIndex& sourceIndex) const override;
    [[nodiscard]] QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;

    [[nodiscard]] int rowCount(const QModelIndex& parent) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent) const override;

    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;

    QString query() const;


private Q_SLOTS:
    void updateMapping();

private:
    QString query_;
    QMap<int, QList<int>> mapping;

    // QString debugPrint(const QAbstractItemModel* model, int row) const;
};
