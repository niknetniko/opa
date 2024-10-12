//
// Created by niko on 9/10/24.
//

#include <QJsonObject>
#include <QJsonDocument>
#include <QStringBuilder>

#include "opa_date.h"

OpaDate::OpaDate(Modifier modifier, Quality quality, const QDate &proleptic, const QDate &endProleptic, bool hasYear,
                 bool hasMonth,
                 bool hasDay, const QString &text) : dateModifier(modifier), dateQuality(quality), proleptic(proleptic),
                                                     endProleptic(endProleptic),
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
    if (endProleptic.isValid()) {
        result[QStringLiteral("prolepticEnd")] = this->endProleptic.toJulianDay();
    }
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

    qDebug() << "Converting from JSON:" << result;
    if (result.isEmpty()) {
        return OpaDate();
    }

    QDate endDate;
    if (result.contains(QStringLiteral("prolepticEnd"))) {
        endDate = QDate::fromJulianDay(result[QStringLiteral("prolepticEnd")].toInteger());
    } else {
        endDate = QDate();
    }

    return OpaDate(
            QVariant(result[QStringLiteral("dateModifier")].toString()).value<Modifier>(),
            QVariant(result[QStringLiteral("dateQuality")].toString()).value<Quality>(),
            QDate::fromJulianDay(result[QStringLiteral("proleptic")].toInteger()),
            endDate,
            result[QStringLiteral("year")].toBool(),
            result[QStringLiteral("month")].toBool(),
            result[QStringLiteral("day")].toBool(),
            result[QStringLiteral("userText")].toString()
    );
}

QDate OpaDate::prolepticRepresentationEnd() const {
    return endProleptic;
}

QString OpaDate::text() const {
    return userText;
}

QString OpaDate::toDisplayText() const {
    // TODO: copy Gramps' date handler system to get this to work better.
    // TODO: support ranges (better or tout court)

    if (!this->text().isEmpty()) {
        return this->text();
    }

    QStringList result;
    if (this->modifier() != NONE) {
        // TODO: allow translating this...
        auto mod = QVariant::fromValue(modifier()).toString().toLower();
        result.append(mod);
    }
    if (this->quality() != EXACT) {
        auto qual = QVariant::fromValue(quality()).toString().toLower();
        result.append(qual);
    }

    // TODO: support not using certain parts of the format.
    QLocale local;
    auto format = local.dateFormat();
    result.append(this->prolepticRepresentation().toString(format));

    return result.join(QStringLiteral(" "));
}

OpaDate OpaDate::fromDisplayText(const QString &text) {
    return OpaDate();
}

QDebug operator<<(QDebug dbg, const OpaDate &date) {
    QDebugStateSaver const saver(dbg);
    dbg.nospace() << date.prolepticRepresentation();
    return dbg;
}
