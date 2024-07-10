#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-magic-numbers"
//
// Created by niko on 8/02/24.
//

#include "formatting.h"

QString format(const QString& pattern, const QVariant &id) {
    return pattern.arg(id.toULongLong(), 4, 10, QLatin1Char('0'));
}

QString format_person_id(const QVariant &id) {
    return format(QString::fromUtf8("P%1"), id);
}

QString format_name_id(const QVariant &id) {
    return format(QString::fromUtf8("N%1"), id);
}


#pragma clang diagnostic pop