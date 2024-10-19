//
// Created by niko on 25/09/24.
//

#include "formatted_identifier_delegate.h"
#include "database/schema.h"

QString format_id(const QString &pattern, const QVariant &id) {
    return pattern.arg(id.toLongLong(), 4, 10, QLatin1Char('0'));
}

FormattedIdentifierDelegate::FormattedIdentifierDelegate(QObject *parent, const QString &pattern): QStyledItemDelegate(
    parent), pattern(pattern) {
}

QString FormattedIdentifierDelegate::displayText(const QVariant &value, const QLocale &locale) const {
    QVariant finalValue = value;
    if (value.canConvert<IntegerPrimaryKey>()) {
        finalValue = format_id(this->pattern, value);
    }

    return QStyledItemDelegate::displayText(finalValue, locale);
}
