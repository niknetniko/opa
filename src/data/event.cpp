#include <QString>
#include <KLocalizedString>

#include "event.h"

//
// Created by niko on 15/10/22.
//
QString Data::Event::Table::typeToDisplay(QString type) {
    if (type == Type::BIRTH) {
        return i18n("Birth");
    }
    if (type == Type::DEATH) {
        return i18n("Death");
    }
    if (type == Type::MARRIAGE) {
        return i18n("Marriage");
    }
    if (type == Type::DIVORCE) {
        return i18n("Divorce");
    }
    if (type == Type::BAPTISM) {
        return i18n("Baptism");
    }
    if (type == Type::CONFIRMATION) {
        return i18n("Confirmation");
    }
    if (type == Type::FIRST_COMMUNION) {
        return i18n("First Communion");
    }
    if (type == Type::FUNERAL) {
        return i18n("Funeral");
    }
    return type;
}
