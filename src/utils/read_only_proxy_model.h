/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QIdentityProxyModel>

/**
 * A proxy model that makes some columns or rows read-only, regardless of what the source models says.
 */
class ReadOnlyProxyModel : public QIdentityProxyModel {
    Q_OBJECT

public:
    explicit ReadOnlyProxyModel(QObject* parent = nullptr);

    /**
     * Add columns to make read-only.
     *
     * @param columns The columns to make read-only.
     */
    void addReadOnlyColumns(const QList<int>& columns);

    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    QList<int> columns;
};
