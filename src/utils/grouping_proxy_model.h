/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QIdentityProxyModel>
#include <QVariant>

/**
 * This model adds a new
 */
class VirtualParentsModel : public QIdentityProxyModel {
    Q_OBJECT

public:
    explicit VirtualParentsModel(QObject* parent = nullptr);

    void setGroupedByColumn(int column);

    void setSourceModel(QAbstractItemModel* sourceModel) override;

    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QModelIndex sibling(int row, int column, const QModelIndex& idx) const override;
    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    int groupedByColumn = -1;
    QVector<QVariant> groups;

    void calculateGroups();
};

QAbstractProxyModel*
createGroupingProxyModel(QAbstractItemModel* original, int groupedByColumn, QObject* parent = nullptr);
