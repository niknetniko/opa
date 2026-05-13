/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "genealogical_date.h"

#include "utils/model_utils.h"

#include <KLocalizedString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStringBuilder>

using namespace Qt::Literals::StringLiterals;

static constexpr QLatin1StringView DATE_FORMAT_FULL{"yyyy-MM-dd"};
static constexpr QLatin1StringView DATE_FORMAT_YEAR_MONTH{"yyyy-MM"};
static constexpr QLatin1StringView TIME_FORMAT{"HH:mm"};

static QString formatDateIso(const GenealogicalDate::DatePoint& p) {
    QString result;
    if (p.year && p.month && p.day) {
        result = p.proleptic.toString(DATE_FORMAT_FULL);
    } else if (p.year && p.month) {
        result = p.proleptic.toString(DATE_FORMAT_YEAR_MONTH);
    } else if (p.year) {
        result = QString::number(p.proleptic.year());
    }
    if (!result.isEmpty() && p.hasTime && p.day) {
        result += u' ' % p.wallTime.toString(TIME_FORMAT);
    }
    return result;
}

static QString formatDateLocalized(const GenealogicalDate::DatePoint& p) {
    QString result;
    if (p.year && p.month && p.day) {
        result = QLocale().toString(p.proleptic, QLocale::LongFormat);
    } else if (p.year && p.month) {
        result = p.proleptic.toString(u"MMMM yyyy"_s);
    } else if (p.year) {
        result = QString::number(p.proleptic.year());
    }
    if (!result.isEmpty() && p.hasTime && p.day) {
        result += u' ' % p.wallTime.toString(TIME_FORMAT);
    }
    return result;
}

GenealogicalDate::GenealogicalDate(
    Modifier modifier,
    Quality quality,
    const QDate& proleptic,
    bool hasYear,
    bool hasMonth,
    bool hasDay,
    QString text
) :
    dateModifier(modifier),
    dateQuality(quality),
    start{.proleptic = proleptic, .year = hasYear, .month = hasMonth, .day = hasDay},
    userText(std::move(text)) {
}

GenealogicalDate GenealogicalDate::makeRange(
    Quality quality,
    const QDate& from,
    bool fromHasYear,
    bool fromHasMonth,
    bool fromHasDay,
    const QDate& to,
    bool toHasYear,
    bool toHasMonth,
    bool toHasDay
) {
    GenealogicalDate d;
    d.dateType = RANGE;
    d.dateQuality = quality;
    d.start = {.proleptic = from, .year = fromHasYear, .month = fromHasMonth, .day = fromHasDay};
    d.end   = {.proleptic = to,   .year = toHasYear,   .month = toHasMonth,   .day = toHasDay};
    return d;
}

GenealogicalDate GenealogicalDate::makeSpan(
    Quality quality,
    const QDate& from,
    bool fromHasYear,
    bool fromHasMonth,
    bool fromHasDay,
    const QDate& to,
    bool toHasYear,
    bool toHasMonth,
    bool toHasDay
) {
    GenealogicalDate d;
    d.dateType = SPAN;
    d.dateQuality = quality;
    d.start = {.proleptic = from, .year = fromHasYear, .month = fromHasMonth, .day = fromHasDay};
    d.end   = {.proleptic = to,   .year = toHasYear,   .month = toHasMonth,   .day = toHasDay};
    return d;
}

GenealogicalDate::DateType GenealogicalDate::type() const {
    return dateType;
}

GenealogicalDate::Quality GenealogicalDate::quality() const {
    return dateQuality;
}

QDate GenealogicalDate::prolepticRepresentation() const {
    return start.proleptic;
}

bool GenealogicalDate::hasMonth() const {
    return start.month;
}

bool GenealogicalDate::hasYear() const {
    return start.year;
}

bool GenealogicalDate::hasDay() const {
    return start.day;
}

GenealogicalDate::DatePoint GenealogicalDate::startPoint() const {
    return start;
}

GenealogicalDate::DatePoint GenealogicalDate::endPoint() const {
    return end;
}

GenealogicalDate::Modifier GenealogicalDate::modifier() const {
    return dateModifier;
}

qint64 GenealogicalDate::sortKey() const {
    return start.proleptic.toJulianDay();
}

bool GenealogicalDate::isNull() const {
    return dateType == SINGLE && !start.proleptic.isValid() && !start.year && !start.month && !start.day
        && userText.isEmpty();
}

bool GenealogicalDate::isValid() const {
    if (isNull()) {
        return false;
    }
    if (dateType == RANGE || dateType == SPAN) {
        return start.proleptic.isValid() && end.proleptic.isValid();
    }
    return start.proleptic.isValid();
}

void GenealogicalDate::setStartTime(const QTime& wallTime) {
    start.wallTime = wallTime;
    start.hasTime = wallTime.isValid();
}

void GenealogicalDate::setEndTime(const QTime& wallTime) {
    end.wallTime = wallTime;
    end.hasTime = wallTime.isValid();
}

QString GenealogicalDate::toDatabaseRepresentation() const {
    QJsonObject result;

    result[u"dateType"_s] = QVariant::fromValue(dateType).toString();
    result[u"dateModifier"_s] = QVariant::fromValue(dateModifier).toString();
    result[u"dateQuality"_s] = QVariant::fromValue(dateQuality).toString();
    result[u"proleptic"_s] = start.proleptic.toJulianDay();
    result[u"year"_s] = start.year;
    result[u"month"_s] = start.month;
    result[u"day"_s] = start.day;
    result[u"endProleptic"_s] = end.proleptic.isValid() ? end.proleptic.toJulianDay() : 0LL;
    result[u"endYear"_s] = end.year;
    result[u"endMonth"_s] = end.month;
    result[u"endDay"_s] = end.day;
    if (start.hasTime) {
        result[u"time"_s] = start.wallTime.toString(TIME_FORMAT);
    }
    if (end.hasTime) {
        result[u"endTime"_s] = end.wallTime.toString(TIME_FORMAT);
    }
    result[u"userText"_s] = userText;

    const QJsonDocument doc(result);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

GenealogicalDate GenealogicalDate::fromDatabaseRepresentation(const QString& text) {
    const QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8());
    const QJsonObject result = doc.object();

    if (result.isEmpty()) {
        return {};
    }

    const auto rawType = result[u"dateType"_s].toString();
    const auto dateType = isValidEnum<DateType>(rawType) ? enumFromString<DateType>(rawType) : SINGLE;

    const auto rawModifier = result[u"dateModifier"_s].toString();
    const auto modifier = isValidEnum<Modifier>(rawModifier) ? enumFromString<Modifier>(rawModifier) : NONE;

    const auto rawQuality = result[u"dateQuality"_s].toString();
    const auto quality = isValidEnum<Quality>(rawQuality) ? enumFromString<Quality>(rawQuality) : EXACT;

    const auto parseTime = [](const QString& s) -> std::pair<QTime, bool> {
        if (s.isEmpty()) return {{}, false};
        auto t = QTime::fromString(s, TIME_FORMAT);
        return {t, t.isValid()};
    };

    const auto [startWallTime, startHasTime] = parseTime(result[u"time"_s].toString());
    const auto [endWallTime, endHasTime] = parseTime(result[u"endTime"_s].toString());

    DatePoint startPoint{
        QDate::fromJulianDay(result[u"proleptic"_s].toInteger()),
        startWallTime,
        result[u"year"_s].toBool(),
        result[u"month"_s].toBool(),
        result[u"day"_s].toBool(),
        startHasTime,
    };
    DatePoint endPoint{
        QDate::fromJulianDay(result[u"endProleptic"_s].toInteger()),
        endWallTime,
        result[u"endYear"_s].toBool(),
        result[u"endMonth"_s].toBool(),
        result[u"endDay"_s].toBool(),
        endHasTime,
    };
    const auto userText = result[u"userText"_s].toString();

    if (dateType == RANGE) {
        auto d = makeRange(
            quality,
            startPoint.proleptic, startPoint.year, startPoint.month, startPoint.day,
            endPoint.proleptic, endPoint.year, endPoint.month, endPoint.day
        );
        d.start.wallTime = startPoint.wallTime;
        d.start.hasTime = startPoint.hasTime;
        d.end.wallTime = endPoint.wallTime;
        d.end.hasTime = endPoint.hasTime;
        d.userText = userText;
        return d;
    }
    if (dateType == SPAN) {
        auto d = makeSpan(
            quality,
            startPoint.proleptic, startPoint.year, startPoint.month, startPoint.day,
            endPoint.proleptic, endPoint.year, endPoint.month, endPoint.day
        );
        d.start.wallTime = startPoint.wallTime;
        d.start.hasTime = startPoint.hasTime;
        d.end.wallTime = endPoint.wallTime;
        d.end.hasTime = endPoint.hasTime;
        d.userText = userText;
        return d;
    }

    GenealogicalDate d{modifier, quality, startPoint.proleptic, startPoint.year, startPoint.month, startPoint.day, userText};
    d.start.wallTime = startPoint.wallTime;
    d.start.hasTime = startPoint.hasTime;
    return d;
}

QString GenealogicalDate::text() const {
    return userText;
}

QString GenealogicalDate::toDisplayText() const {
    if (dateType == RANGE) {
        return u"between "_s % formatDateIso(start) % u" and "_s % formatDateIso(end);
    }
    if (dateType == SPAN) {
        return u"from "_s % formatDateIso(start) % u" to "_s % formatDateIso(end);
    }

    QStringList result;
    if (modifier() != NONE) {
        result.append(QVariant::fromValue(modifier()).toString().toLower());
    }
    if (quality() != EXACT) {
        result.append(QVariant::fromValue(quality()).toString().toLower());
    }
    result.append(formatDateIso(start));
    return result.join(u" "_s);
}

QString GenealogicalDate::toLocalizedText() const {
    static const QHash<Modifier, QString> modifierLabels = {
        {BEFORE, i18nc("date modifier", "before")},
        {AFTER,  i18nc("date modifier", "after")},
        {ABOUT,  i18nc("date modifier", "about")},
        {DURING, i18nc("date modifier", "during")},
    };
    static const QHash<Quality, QString> qualityLabels = {
        {ESTIMATED,  i18nc("date quality", "estimated")},
        {CALCULATED, i18nc("date quality", "calculated")},
    };

    if (dateType == RANGE) {
        return i18nc(
            "date range display",
            "between %1 and %2",
            formatDateLocalized(start),
            formatDateLocalized(end)
        );
    }
    if (dateType == SPAN) {
        return i18nc(
            "date span display",
            "from %1 to %2",
            formatDateLocalized(start),
            formatDateLocalized(end)
        );
    }

    QStringList result;
    if (modifier() != NONE) {
        result.append(modifierLabels.value(modifier()));
    }
    if (quality() != EXACT) {
        result.append(qualityLabels.value(quality()));
    }
    result.append(formatDateLocalized(start));
    return result.join(u" "_s);
}

// Parse a partial ISO 8601 date string (with optional trailing " HH:mm"); returns DatePoint.
static GenealogicalDate::DatePoint parsePartialDate(const QString& text) {
    static const QRegularExpression timeRx(u" (\\d{2}:\\d{2})$"_s);

    QTime wallTime;
    bool hasTime = false;
    QString datePart = text;
    if (const auto m = timeRx.match(text); m.hasMatch()) {
        wallTime = QTime::fromString(m.captured(1), TIME_FORMAT);
        hasTime = wallTime.isValid();
        datePart = text.left(text.length() - m.capturedLength());
    }

    // Full date
    auto date = QDate::fromString(datePart, DATE_FORMAT_FULL);
    if (date.isValid()) {
        return {date, wallTime, true, true, true, hasTime};
    }
    // Year+month (time ignored for partial dates without a day)
    date = QDate::fromString(datePart + u"-01"_s, DATE_FORMAT_FULL);
    if (date.isValid()) {
        return {date, {}, true, true, false, false};
    }
    // Year-only
    bool ok = false;
    const int yearVal = datePart.toInt(&ok);
    if (ok && yearVal != 0) {
        return {QDate(yearVal, 1, 1), {}, true, false, false, false};
    }
    return {};
}

GenealogicalDate GenealogicalDate::fromDisplayText(const QString& text) {
    if (text.isEmpty()) {
        return {};
    }

    // Range: "between DATE and DATE"
    if (text.startsWith(u"between "_s)) {
        const auto inner = text.mid(8);
        const int andPos = inner.indexOf(u" and "_s);
        if (andPos >= 0) {
            const auto fromPoint = parsePartialDate(inner.left(andPos));
            const auto toPoint = parsePartialDate(inner.mid(andPos + 5));
            if (fromPoint.proleptic.isValid() && toPoint.proleptic.isValid()) {
                auto d = makeRange(
                    EXACT,
                    fromPoint.proleptic, fromPoint.year, fromPoint.month, fromPoint.day,
                    toPoint.proleptic, toPoint.year, toPoint.month, toPoint.day
                );
                d.start.wallTime = fromPoint.wallTime; d.start.hasTime = fromPoint.hasTime;
                d.end.wallTime = toPoint.wallTime;     d.end.hasTime = toPoint.hasTime;
                return d;
            }
        }
        return {};
    }

    // Span: "from DATE to DATE"
    if (text.startsWith(u"from "_s)) {
        const auto inner = text.mid(5);
        const int toPos = inner.indexOf(u" to "_s);
        if (toPos >= 0) {
            const auto fromPoint = parsePartialDate(inner.left(toPos));
            const auto toPoint = parsePartialDate(inner.mid(toPos + 4));
            if (fromPoint.proleptic.isValid() && toPoint.proleptic.isValid()) {
                auto d = makeSpan(
                    EXACT,
                    fromPoint.proleptic, fromPoint.year, fromPoint.month, fromPoint.day,
                    toPoint.proleptic, toPoint.year, toPoint.month, toPoint.day
                );
                d.start.wallTime = fromPoint.wallTime; d.start.hasTime = fromPoint.hasTime;
                d.end.wallTime = toPoint.wallTime;     d.end.hasTime = toPoint.hasTime;
                return d;
            }
        }
        return {};
    }

    auto parts = text.split(u" "_s);

    auto modifier = NONE;
    auto quality = EXACT;

    const auto rawPart1 = parts.constFirst().toUpper();
    if (isValidEnum<Modifier>(rawPart1)) {
        modifier = enumFromString<Modifier>(rawPart1);
        parts.removeFirst();
    }

    if (!parts.isEmpty()) {
        const auto rawPart2 = parts.constFirst().toUpper();
        if (isValidEnum<Quality>(rawPart2)) {
            quality = enumFromString<Quality>(rawPart2);
            parts.removeFirst();
        }
    }

    const auto p = parsePartialDate(parts.join(u" "_s));
    if (!p.proleptic.isValid() && !p.year) {
        return {};
    }

    GenealogicalDate d{modifier, quality, p.proleptic, p.year, p.month, p.day, QStringLiteral()};
    d.start.wallTime = p.wallTime;
    d.start.hasTime = p.hasTime;
    return d;
}

QDebug operator<<(QDebug dbg, const GenealogicalDate& date) {
    const QDebugStateSaver saver(dbg);
    dbg.nospace() << date.toDatabaseRepresentation();
    return dbg;
}
