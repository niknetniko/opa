#include <QString>
#include <KLocalizedString>
#include "person.h"
#include "names/names_overview_view.h"

QString Data::Person::Sex::toDisplay(const QString &sex) {
    if (sex == Person::Sex::MALE) {
        return i18n("Man");
    }
    if (sex == Person::Sex::FEMALE) {
        return i18n("Vrouw");
    }
    if (sex == Person::Sex::UNKNOWN) {
        return i18n("Onbekend");
    }
    return sex;
}

QString Data::Person::Sex::toIcon(const QString &sex) {
    if (sex == Person::Sex::MALE) {
        return QStringLiteral("♂");
    }
    if (sex == Person::Sex::FEMALE) {
        return QStringLiteral("♀");
    }
    if (sex == Person::Sex::UNKNOWN) {
        return QStringLiteral("?");
    }
    return QStringLiteral("⚥");
}

QVariant DisplayNameProxyModel::extraColumnData(const QModelIndex &parent, int row, int extraColumn, int role) const {
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (extraColumn == 0) {
            // TODO: do not hard code this
            auto titles = this->index(row, 1, parent).data(role).toString();
            auto givenNames = this->index(row, 2, parent).data(role).toString();
            auto prefix = this->index(row, 3, parent).data(role).toString();
            auto surname = this->index(row, 4, parent).data(role).toString();
            return Names::construct_display_name(titles, givenNames, prefix, surname);
        }
    }
    return {};
}

DisplayNameProxyModel::DisplayNameProxyModel(QObject *parent) : KExtraColumnsProxyModel(parent) {
    this->appendColumn(i18n("Naam"));
}
