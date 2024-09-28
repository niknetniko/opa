#include <QString>
#include <KLocalizedString>
#include "person.h"
#include "names/names_table_view.h"

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

QVariant DisplayNameProxyModel::extraColumnData(const QModelIndex &parent, int row, int /*extraColumn*/, int role) const {
    auto titles = this->index(row, 1, parent).data(role).toString();
    auto givenNames = this->index(row, 2, parent).data(role).toString();
    auto prefix = this->index(row, 3, parent).data(role).toString();
    auto surname = this->index(row, 4, parent).data(role).toString();
    return Names::construct_display_name(titles, givenNames, prefix, surname);
}

DisplayNameProxyModel::DisplayNameProxyModel(QObject *parent) : KExtraColumnsProxyModel(parent) {
    this->appendColumn(i18n("Naam"));
}
