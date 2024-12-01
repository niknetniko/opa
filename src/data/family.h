/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "database/schema.h"
#include "event.h"

#include <QAbstractProxyModel>

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
    static constexpr int TITLES = 7;
    static constexpr int GIVEN_NAMES = 8;
    static constexpr int PREFIX = 9;
    static constexpr int SURNAME = 10;
    static constexpr int DISPLAY_NAME = 11;

    explicit FamilyProxyModel(IntegerPrimaryKey person, QObject* parent = nullptr);

    [[nodiscard]] QModelIndex parent(const QModelIndex& child) const override;
    [[nodiscard]] bool hasChildren(const QModelIndex& parent) const override;

    [[nodiscard]] QModelIndex mapFromSource(const QModelIndex& sourceIndex) const override;
    [[nodiscard]] QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;

    [[nodiscard]] bool hasBastardChildren() const;
public Q_SLOTS:
    void resetAndLoadData();

private Q_SLOTS:
    void updateMapping();

private:
    IntegerPrimaryKey person;
    QString query_;
    QMap<int, QList<int>> mapping;

    [[nodiscard]] bool isBastardParentRow(const QModelIndex& index) const;
    [[nodiscard]] bool hasBastardParent(const QModelIndex& parent) const;
};

namespace FamilyDisplayModel {
    constexpr int TYPE = 0;
    constexpr int DATE = 1;
    constexpr int DISPLAY_NAME = 2;
    constexpr int ROLE = 3;
    constexpr int EVENT_ID = 4;
}

class AncestorQueryModel : public QSqlQueryModel {
    Q_OBJECT
public:
    static constexpr int CHILD_ID = 0;
    static constexpr int FATHER_ID = 1;
    static constexpr int MOTHER_ID = 2;
    static constexpr int VISITED = 3;
    static constexpr int LEVEL = 4;
    static constexpr int TITLES = 5;
    static constexpr int GIVEN_NAMES = 6;
    static constexpr int PREFIX = 7;
    static constexpr int SURNAME = 8;

    explicit AncestorQueryModel(IntegerPrimaryKey person, QObject* parent = nullptr);

public Q_SLOTS:
    void resetAndLoadData();

private:
    IntegerPrimaryKey person;
    QString query_;
};

namespace AncestorDisplayModel {
    static constexpr int CHILD_ID = 0;
    static constexpr int FATHER_ID = 1;
    static constexpr int MOTHER_ID = 2;
    static constexpr int VISITED = 3;
    static constexpr int LEVEL = 4;
    static constexpr int DISPLAY_NAME = 5;
}
