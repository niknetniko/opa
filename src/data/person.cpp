#include <QString>
#include <KLocalizedString>
#include "person.h"

QString Data::Person::Sex::toDisplay(const QString &sex) {
    if (sex == Person::Sex::MALE) {
        return i18n("Male");
    }
    if (sex == Person::Sex::FEMALE) {
        return i18n("Female");
    }
    if (sex == Person::Sex::UNKNOWN) {
        return i18n("Unknown");
    }
    return sex;
}

QString Data::Person::Sex::toIcon(const QString &sex) {
    if (sex == Person::Sex::MALE) {
        return QString::fromUtf8("♂");
    }
    if (sex == Person::Sex::FEMALE) {
        return QString::fromUtf8("♀");
    }
    if (sex == Person::Sex::UNKNOWN) {
        return QString::fromUtf8("?");
    }
    return QString::fromUtf8("⚥");
}
