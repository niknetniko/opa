/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"
#include "utils/formatted_identifier_delegate.h"

#include <QAbstractProxyModel>
#include <QSqlRecord>

namespace Ui {
    class PersonDetailView;
}

class PersonDetailView : public QFrame {
    Q_OBJECT

public:
    explicit PersonDetailView(IntegerPrimaryKey id, QWidget* parent);

    [[nodiscard]] bool hasId(IntegerPrimaryKey id) const;

    [[nodiscard]] QString getDisplayName() const;

    ~PersonDetailView() override;

public Q_SLOTS:
    /**
     * Populate the UI with the data from the field.
     */
    void populate();

Q_SIGNALS:
    /**
     * Emitted when the UI has been re-populated.
     */
    void dataChanged(IntegerPrimaryKey id);

private:
    QAbstractProxyModel* model;
    Ui::PersonDetailView* ui;
};
