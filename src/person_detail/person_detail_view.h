/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QAbstractItemModel>
#include <QFrame>


class QAbstractItemModel;
class QAbstractProxyModel;

namespace Ui {
    class PersonDetailView;
}

struct BirthInformation {
    QString symbol;
    QString date;
};

struct DeathInformation {
    QString symbol;
    QString date;
};

BirthInformation constructBirthText(const QAbstractItemModel* model);

DeathInformation constructDeathText(const QAbstractItemModel* birthModel, const QAbstractItemModel* deathModel);

class PersonDetailView : public QFrame {
    Q_OBJECT

public:
    explicit PersonDetailView(IntegerPrimaryKey id, QWidget* parent);

    [[nodiscard]] bool hasId(IntegerPrimaryKey id) const;

    [[nodiscard]] QString getDisplayName() const;

    [[nodiscard]] IntegerPrimaryKey getId() const;

    ~PersonDetailView() override;

public Q_SLOTS:
    /**
     * Populate the UI with the data from the field.
     */
    void populateName();
    void populateDates() const;

Q_SIGNALS:
    /**
     * Emitted when the UI has been re-populated.
     */
    void nameChanged(IntegerPrimaryKey id);

private:
    QAbstractProxyModel* model;
    QAbstractItemModel* birthModel;
    QAbstractItemModel* deathModel;
    Ui::PersonDetailView* ui;
    IntegerPrimaryKey id;
};
