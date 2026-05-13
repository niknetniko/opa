/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QDate>
#include <QTime>

/**
 * Represents dates in Opa.
 *
 * A date consists of the following:
 *
 * - A type (single, range, span).
 * - A period modifier (before, after, about, during) — applies to SINGLE dates only.
 * - A quality modifier (exact, estimated, calculated).
 * - A proleptic Gregorian calendar representation.
 * - Information about which parts are available.
 * - A free-form text version.
 *
 * For RANGE and SPAN dates, a second proleptic Gregorian calendar representation is given:
 * - RANGE: "between DATE_1 and DATE_2"
 * - SPAN: "from DATE_1 to DATE_2"
 *
 * When a date does not have certain components, they are represented by 1 in the proleptic
 * representation (e.g., year-only uses QDate(year, 1, 1)). This makes sorting easier.
 * The hasYear/hasMonth/hasDay flags indicate which components are meaningful.
 *
 * A default-constructed GenealogicalDate is null (isNull() == true).
 *
 * toDisplayText() produces an ISO 8601 interchange format parseable by fromDisplayText().
 * toLocalizedText() produces a locale-friendly human-readable string for display in views.
 *
 * TODO: support multiple calendars
 */
class GenealogicalDate {
    Q_GADGET

public:
    enum DateType { SINGLE, RANGE, SPAN };

    Q_ENUM(DateType);

    enum Modifier { NONE, BEFORE, AFTER, ABOUT, DURING };

    Q_ENUM(Modifier);

    enum Quality { EXACT, ESTIMATED, CALCULATED };

    Q_ENUM(Quality);

    GenealogicalDate() = default;

    ~GenealogicalDate() = default;

    GenealogicalDate(const GenealogicalDate&) = default;

    GenealogicalDate(
        Modifier modifier,
        Quality quality,
        const QDate& proleptic,
        bool hasYear,
        bool hasMonth,
        bool hasDay,
        QString text
    );

    GenealogicalDate& operator=(const GenealogicalDate&) = default;

    bool operator==(const GenealogicalDate&) const = default;

    struct DatePoint {
        QDate proleptic;
        QTime wallTime;          // recorded wall time only — not used for sorting/calculation
        bool year = false;
        bool month = false;
        bool day = false;
        bool hasTime = false;    // only meaningful when day is true

        bool operator==(const DatePoint&) const = default;
    };

    static GenealogicalDate makeRange(
        Quality quality,
        const QDate& from,
        bool fromHasYear,
        bool fromHasMonth,
        bool fromHasDay,
        const QDate& to,
        bool toHasYear,
        bool toHasMonth,
        bool toHasDay
    );

    static GenealogicalDate makeSpan(
        Quality quality,
        const QDate& from,
        bool fromHasYear,
        bool fromHasMonth,
        bool fromHasDay,
        const QDate& to,
        bool toHasYear,
        bool toHasMonth,
        bool toHasDay
    );

    [[nodiscard]] DateType type() const;

    // Convenience accessors for the start (or sole) date.
    [[nodiscard]] bool hasMonth() const;
    [[nodiscard]] bool hasYear() const;
    [[nodiscard]] bool hasDay() const;
    [[nodiscard]] QDate prolepticRepresentation() const;
    [[nodiscard]] DatePoint startPoint() const;

    // Accessors for the end date (only meaningful for RANGE/SPAN).
    [[nodiscard]] DatePoint endPoint() const;

    [[nodiscard]] Quality quality() const;

    [[nodiscard]] Modifier modifier() const;

    [[nodiscard]] qint64 sortKey() const;

    [[nodiscard]] QString toDatabaseRepresentation() const;

    [[nodiscard]] QString toDisplayText() const;

    [[nodiscard]] QString toLocalizedText() const;

    [[nodiscard]] QString text() const;

    [[nodiscard]] bool isNull() const;

    [[nodiscard]] bool isValid() const;

    // Pass an invalid QTime() to clear wall time.
    void setStartTime(const QTime& wallTime);
    void setEndTime(const QTime& wallTime);

    static GenealogicalDate fromDatabaseRepresentation(const QString& text);

    static GenealogicalDate fromDisplayText(const QString& text);

private:

    DateType dateType = SINGLE;
    Modifier dateModifier = NONE;
    Quality dateQuality = EXACT;
    DatePoint start;
    DatePoint end;
    QString userText;
};

Q_DECLARE_METATYPE(GenealogicalDate);

QDebug operator<<(QDebug dbg, const GenealogicalDate& date);
