//
// Created by niko on 9/10/24.
//

#include <QJsonObject>
#include <QJsonDocument>
#include "opa_date.h"

OpaDate::OpaDate(Modifier modifier, Quality quality, const QDate &proleptic, bool hasYear, bool hasMonth,
                 bool hasDay, const QString &text) : dateModifier(modifier), dateQuality(quality), proleptic(proleptic),
                                                     year(hasYear), month(hasMonth), day(hasDay), userText(text) {}

OpaDate::Quality OpaDate::quality() const {
    return this->dateQuality;
}

QDate OpaDate::prolepticRepresentation() const {
    return this->proleptic;
}

bool OpaDate::hasMonth() const {
    return month;
}

bool OpaDate::hasYear() const {
    return year;
}

bool OpaDate::hasDay() const {
    return day;
}

OpaDate::Modifier OpaDate::modifier() const {
    return dateModifier;
}

QString OpaDate::toDatabaseRepresentation() const {
    QJsonObject result;

    result[QStringLiteral("dateModifier")] = QVariant::fromValue(this->dateModifier).toString();
    result[QStringLiteral("dateQuality")] = QVariant::fromValue(this->dateQuality).toString();
    result[QStringLiteral("proleptic")] = this->proleptic.toJulianDay();
    result[QStringLiteral("year")] = this->year;
    result[QStringLiteral("month")] = this->month;
    result[QStringLiteral("day")] = this->day;
    result[QStringLiteral("userText")] = this->userText;

    QJsonDocument doc(result);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

OpaDate OpaDate::fromDatabaseRepresentation(const QString &text) {
    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8());
    QJsonObject result = doc.object();
    return OpaDate(
            QVariant(result[QStringLiteral("dateModifier")].toString()).value<Modifier>(),
            QVariant(result[QStringLiteral("dateQuality")].toString()).value<Quality>(),
            QDate::fromJulianDay(result[QStringLiteral("proleptic")].toInteger()),
            result[QStringLiteral("year")].toBool(),
            result[QStringLiteral("month")].toBool(),
            result[QStringLiteral("day")].toBool(),
            result[QStringLiteral("userText")].toString()
    );
}

QDebug operator<<(QDebug dbg, const OpaDate &date) {
    QDebugStateSaver const saver(dbg);
    dbg.nospace() << date.prolepticRepresentation();
    return dbg;
}
