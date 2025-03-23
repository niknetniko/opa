/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <KExtraColumnsProxyModel>
#include <QIdentityProxyModel>
#include <QVariant>


/**
 * This model adds a new
 */
class VirtualParentsModel : public KExtraColumnsProxyModel {
    Q_OBJECT

public:
    explicit VirtualParentsModel(QObject* parent = nullptr);

    void setGroupedByColumn(int column);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
    [[nodiscard]] QVariant
    extraColumnData(const QModelIndex& parent, int row, int extraColumn, int role) const override;

private:
    int groupedByColumn = -1;
    QVector<QVariant> groups;

    void calculateGroups();
};

QAbstractProxyModel*
createGroupingProxyModel(QAbstractItemModel* original, int groupedByColumn, QObject* parent = nullptr);
