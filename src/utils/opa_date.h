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
class OpaDate {
    Q_GADGET

public:
    enum Modifier { NONE, BEFORE, AFTER, DURING };

    Q_ENUM(Modifier);

    enum Quality { EXACT, ESTIMATED, CALCULATED };

    Q_ENUM(Quality);

    OpaDate() = default;

    ~OpaDate() = default;

    OpaDate(const OpaDate &) = default;

    OpaDate(
        Modifier modifier,
        Quality quality,
        const QDate &proleptic,
        const QDate &endProleptic,
        bool hasYear,
        bool hasMonth,
        bool hasDay,
        const QString &text
    );

    OpaDate &operator=(const OpaDate &) = default;

    bool hasMonth() const;

    bool hasYear() const;

    bool hasDay() const;

    QDate prolepticRepresentation() const;

    QDate prolepticRepresentationEnd() const;

    Quality quality() const;

    Modifier modifier() const;

    QString toDatabaseRepresentation() const;

    QString toDisplayText() const;

    QString text() const;

    static OpaDate fromDatabaseRepresentation(const QString &text);

    static OpaDate fromDisplayText(const QString &text);

private:
    Modifier dateModifier = NONE;
    Quality dateQuality = EXACT;
    QDate proleptic;
    QDate endProleptic;
    bool year;
    bool month;
    bool day;
    QString userText;
};

Q_DECLARE_METATYPE(OpaDate);

QDebug operator<<(QDebug dbg, const OpaDate &date);
