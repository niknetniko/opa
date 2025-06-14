/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QStyledItemDelegate>


class QItemSelection;

QString format_id(const QString& pattern, const QVariant& id);

class FormattedIdentifierDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    static constexpr auto PERSON = QLatin1String("P%1");
    static constexpr auto NAME = QLatin1String("N%1");
    static constexpr auto EVENT = QLatin1String("E%1");
    static constexpr auto SOURCE = QLatin1String("S%1");

    explicit FormattedIdentifierDelegate(QObject* parent, QString pattern);

    [[nodiscard]] QString displayText(const QVariant& value, const QLocale& locale) const override;

private:
    QString pattern;
};

IntegerPrimaryKey getIdFromSelection(const QItemSelection& selected, const QAbstractItemModel* model, int column);
