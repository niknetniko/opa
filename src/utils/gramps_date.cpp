//
// Created by niko on 19/05/23.
//

#include "gramps_date.h"

QDebug operator<<(QDebug dbg, const GrampsDate& date) {
    QDebugStateSaver const saver(dbg);
    auto string_year = date.year.has_value() ? QString::number(date.year.value()) : "?";
    auto string_month = date.month.has_value() ? QString::number(date.month.value()) : "?";
    auto string_day = date.day.has_value() ? QString::number(date.day.value()) : "?";
    dbg.nospace() << string_year << "-" << string_month << "-" << string_day;
    return dbg;
}
