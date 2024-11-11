/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QDate>

/**
 * Represents dates in Opa.
 *
 * A date consists of the following:
 *
 * - A period modifier (before, after, during).
 * - A quality modifier (exact, estimated, calculated).
 * - A proleptic Gregorian calendar representation.
 * - Information about which parts are available.
 * - A free-form text version.
 *
 * Additionally, a second proleptic Gregorian calendar representation can be given,
 * which allows representing a range.
 *
 * When a date does not have certain components, they are represented by 1.
 * For example, if there is no year, it will be added with year 1 in the proleptic representation.
 * This is to make sorting easier and less cumbersome everywhere.
 *
 * TODO: support multiple calendars
 * TODO: support parsing from free-form text
 * TODO: support generating human-readable text from this
 */
class GenealogicalDate {
    Q_GADGET

public:
    enum Modifier { NONE, BEFORE, AFTER, DURING };

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

    [[nodiscard]] bool hasMonth() const;

    [[nodiscard]] bool hasYear() const;

    [[nodiscard]] bool hasDay() const;

    [[nodiscard]] QDate prolepticRepresentation() const;

    [[nodiscard]] Quality quality() const;

    [[nodiscard]] Modifier modifier() const;

    [[nodiscard]] QString toDatabaseRepresentation() const;

    [[nodiscard]] QString toDisplayText() const;

    [[nodiscard]] QString text() const;

    static GenealogicalDate fromDatabaseRepresentation(const QString& text);

    static GenealogicalDate fromDisplayText(const QString& text);

private:
    Modifier dateModifier = NONE;
    Quality dateQuality = EXACT;
    QDate proleptic;
    bool year = true;
    bool month = true;
    bool day = true;
    QString userText;
};

Q_DECLARE_METATYPE(GenealogicalDate);

QDebug operator<<(QDebug dbg, const GenealogicalDate& date);
