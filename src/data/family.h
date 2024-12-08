/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "database/schema.h"
#include "event.h"

#include <QAbstractProxyModel>
#include <QSqlQueryModel>

/**
 * Renders all spouses and children of a single person into a tree view of families.
 */
class FamilyProxyModel : public QAbstractProxyModel {
    Q_OBJECT

public:
    static constexpr int EVENT_TYPE = 0;
    static constexpr int EVENT_TYPE_ID = 1;
    static constexpr int PERSON_ID = 2;
    static constexpr int PARTNER_ID = 3;
    static constexpr int EVENT_ID = 4;
    static constexpr int DATE = 5;
    static constexpr int TITLES = 6;
    static constexpr int GIVEN_NAMES = 7;
    static constexpr int PREFIX = 8;
    static constexpr int SURNAME = 9;

    explicit FamilyProxyModel(IntegerPrimaryKey person, QObject* parent = nullptr);

    [[nodiscard]] QModelIndex parent(const QModelIndex& child) const override;

    [[nodiscard]] QModelIndex mapFromSource(const QModelIndex& sourceIndex) const override;
    [[nodiscard]] QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
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
    constexpr int PERSON_ID = 2;
    constexpr int DISPLAY_NAME = 3;
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

class ParentQueryModel : public QSqlQueryModel {
    Q_OBJECT

public:
    static constexpr int PERSON_ID = 0;
    static constexpr int ROLE_ID = 1;
    static constexpr int ROLE = 2;
    static constexpr int TITLES = 3;
    static constexpr int GIVEN_NAMES = 4;
    static constexpr int PREFIX = 5;
    static constexpr int SURNAME = 6;

    explicit ParentQueryModel(IntegerPrimaryKey person, QObject* parent = nullptr);

public Q_SLOTS:
    void resetAndLoadData();

private:
    IntegerPrimaryKey person;
    QString query_;
};

namespace DisplayParentModel {
    static constexpr int ROLE = 0;
    static constexpr int PERSON_ID = 1;
    static constexpr int DISPLAY_NAME = 2;
}
