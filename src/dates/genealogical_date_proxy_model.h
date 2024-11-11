
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QIdentityProxyModel>

/**
 * Class the handle a column where an OpaDate is saved as JSON in some column.
 * The model ignores empty strings or null values but will error on other invalid values.
 *
 * If you need to access the underlying data, query the model using the RawDateRole role.
 *
 * You need to set which column has the date information, otherwise this model does nothing.
 */
class GenealogicalDateProxyModel : public QIdentityProxyModel {
    Q_OBJECT

public:
    static constexpr int RawDateRole = Qt::UserRole + 20;

    explicit GenealogicalDateProxyModel(QObject* parent = nullptr);

    /**
     * Set which column contains the date.
     */
    void setDateColumn(int column);

    /**
     * Get which column contains the date or -1 if no column contains a date.
     */
    [[nodiscard]] int dateColumn() const;

    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

private:
    int dateColumn_ = -1;
};
