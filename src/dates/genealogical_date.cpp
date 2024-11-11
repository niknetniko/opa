/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "genealogical_date.h"

#include "utils/model_utils_find_source_model_of_type.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QStringBuilder>

GenealogicalDate::GenealogicalDate(
    Modifier modifier, Quality quality, const QDate& proleptic, bool hasYear, bool hasMonth, bool hasDay, QString text
) :
    dateModifier(modifier),
    dateQuality(quality),
    proleptic(proleptic),
    year(hasYear),
    month(hasMonth),
    day(hasDay),
    userText(std::move(text)) {
}

GenealogicalDate::Quality GenealogicalDate::quality() const {
    return this->dateQuality;
}

QDate GenealogicalDate::prolepticRepresentation() const {
    return this->proleptic;
}

bool GenealogicalDate::hasMonth() const {
    return month;
}

bool GenealogicalDate::hasYear() const {
    return year;
}

bool GenealogicalDate::hasDay() const {
    return day;
}

GenealogicalDate::Modifier GenealogicalDate::modifier() const {
    return dateModifier;
}

QString GenealogicalDate::toDatabaseRepresentation() const {
    QJsonObject result;

    result[QStringLiteral("dateModifier")] = QVariant::fromValue(this->dateModifier).toString();
    result[QStringLiteral("dateQuality")] = QVariant::fromValue(this->dateQuality).toString();
    result[QStringLiteral("proleptic")] = this->proleptic.toJulianDay();
    result[QStringLiteral("year")] = this->year;
    result[QStringLiteral("month")] = this->month;
    result[QStringLiteral("day")] = this->day;
    result[QStringLiteral("userText")] = this->userText;

    const QJsonDocument doc(result);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

GenealogicalDate GenealogicalDate::fromDatabaseRepresentation(const QString& text) {
    const QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8());
    QJsonObject result = doc.object();

    if (result.isEmpty()) {
        return {};
    }

    return {
        QVariant(result[QStringLiteral("dateModifier")].toString()).value<Modifier>(),
        QVariant(result[QStringLiteral("dateQuality")].toString()).value<Quality>(),
        QDate::fromJulianDay(result[QStringLiteral("proleptic")].toInteger()),
        result[QStringLiteral("year")].toBool(),
        result[QStringLiteral("month")].toBool(),
        result[QStringLiteral("day")].toBool(),
        result[QStringLiteral("userText")].toString()
    };
}

QString GenealogicalDate::text() const {
    return userText;
}

QString GenealogicalDate::toDisplayText() const {
    QStringList result;
    if (this->modifier() != NONE) {
        auto mod = QVariant::fromValue(modifier()).toString().toLower();
        result.append(mod);
    }
    if (this->quality() != EXACT) {
        auto theQuality = QVariant::fromValue(quality()).toString().toLower();
        result.append(theQuality);
    }

    const QLocale local;
    auto format = local.dateFormat();
    result.append(this->prolepticRepresentation().toString(format));

    return result.join(QStringLiteral(" "));
}

GenealogicalDate GenealogicalDate::fromDisplayText(const QString& text) {
    auto parts = text.split(QStringLiteral(" "));
    if (parts.empty()) {
        return {};
    }

    qDebug() << "Parts are" << parts;

    auto modifier = NONE;
    auto quality = EXACT;
    auto dateRemainder = QStringLiteral();

    const auto rawPart1 = parts.constFirst().toUpper();
    if (isValidEnum<Modifier>(rawPart1)) {
        modifier = enumFromString<Modifier>(rawPart1);
        parts.removeFirst();
    }

    const auto rawPart2 = parts.constFirst().toUpper();
    if (isValidEnum<Quality>(rawPart2)) {
        quality = enumFromString<Quality>(rawPart2);
        parts.removeFirst();
    }

    dateRemainder = parts.join(QStringLiteral(" "));

    const QLocale local;
    auto format = local.dateFormat();
    auto date = QDate::fromString(dateRemainder, format);

    return {modifier, quality, date, true, true, true, QStringLiteral()};
}

QDebug operator<<(QDebug dbg, const GenealogicalDate& date) {
    const QDebugStateSaver saver(dbg);
    dbg.nospace() << date.toDatabaseRepresentation();
    return dbg;
}
