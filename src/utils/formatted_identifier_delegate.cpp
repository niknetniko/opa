/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "formatted_identifier_delegate.h"

#include "database/schema.h"

#include <utility>

QString format_id(const QString& pattern, const QVariant& id) {
    return pattern.arg(id.toLongLong(), 4, 10, QLatin1Char('0'));
}

FormattedIdentifierDelegate::FormattedIdentifierDelegate(QObject* parent, QString pattern) :
    QStyledItemDelegate(parent),
    pattern(std::move(pattern)) {
}

QString FormattedIdentifierDelegate::displayText(const QVariant& value, const QLocale& locale) const {
    QVariant finalValue = value;
    if (value.canConvert<IntegerPrimaryKey>()) {
        finalValue = format_id(this->pattern, value);
    }

    return QStyledItemDelegate::displayText(finalValue, locale);
}
