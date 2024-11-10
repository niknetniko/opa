/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "opa_date.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QStringBuilder>
#include <utility>

OpaDate::OpaDate(
    Modifier modifier,
    Quality quality,
    const QDate& proleptic,
    const QDate& endProleptic,
    bool hasYear,
    bool hasMonth,
    bool hasDay,
    QString text
) :
    dateModifier(modifier),
    dateQuality(quality),
    proleptic(proleptic),
    endProleptic(endProleptic),
    year(hasYear),
    month(hasMonth),
    day(hasDay),
    userText(std::move(text)) {
}

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

    const QJsonDocument doc(result);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

OpaDate OpaDate::fromDatabaseRepresentation(const QString& text) {
    const QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8());
    QJsonObject result = doc.object();

    if (result.isEmpty()) {
        return {};
    }

    QDate endDate;
    if (result.contains(QStringLiteral("prolepticEnd"))) {
        endDate = QDate::fromJulianDay(result[QStringLiteral("prolepticEnd")].toInteger());
    } else {
        endDate = QDate();
    }

    return {
        QVariant(result[QStringLiteral("dateModifier")].toString()).value<Modifier>(),
        QVariant(result[QStringLiteral("dateQuality")].toString()).value<Quality>(),
        QDate::fromJulianDay(result[QStringLiteral("proleptic")].toInteger()),
        endDate,
        result[QStringLiteral("year")].toBool(),
        result[QStringLiteral("month")].toBool(),
        result[QStringLiteral("day")].toBool(),
        result[QStringLiteral("userText")].toString()
    };
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
        auto theQuality = QVariant::fromValue(quality()).toString().toLower();
        result.append(theQuality);
    }

    // TODO: support not using certain parts of the format.
    const QLocale local;
    auto format = local.dateFormat();
    result.append(this->prolepticRepresentation().toString(format));

    return result.join(QStringLiteral(" "));
}

OpaDate OpaDate::fromDisplayText([[maybe_unused]] const QString& text) {
    return {};
}

QDebug operator<<(QDebug dbg, const OpaDate& date) {
    const QDebugStateSaver saver(dbg);
    dbg.nospace() << date.prolepticRepresentation();
    return dbg;
}
