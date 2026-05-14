/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeConst
#include "dates/genealogical_date.h"

#include <QSqlDatabase>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestOpaDate : public QObject {
    Q_OBJECT

    const GenealogicalDate date =
        GenealogicalDate(GenealogicalDate::DURING, GenealogicalDate::EXACT, QDate(2001, 7, 4), true, true, true, u""_s);

    const QString json =
        u"{\"dateModifier\":\"DURING\",\"dateQuality\":\"EXACT\",\"dateType\":\"SINGLE\",\"day\":true,\"endDay\":false,\"endMonth\":false,\"endProleptic\":0,\"endYear\":false,\"month\":true,\"proleptic\":2452095,\"userText\":\"\",\"year\":true}"_s;

private Q_SLOTS:
    void testJsonEncoding() {
        const auto generated = date.toDatabaseRepresentation();
        QCOMPARE(generated, json);
    }

    void testJsonDecoding() {
        auto decoded = GenealogicalDate::fromDatabaseRepresentation(json);
        QCOMPARE(date.modifier(), decoded.modifier());
        QCOMPARE(date.quality(), decoded.quality());
        QCOMPARE(date.prolepticRepresentation(), decoded.prolepticRepresentation());
        QCOMPARE(date.hasDay(), decoded.hasDay());
        QCOMPARE(date.hasMonth(), decoded.hasMonth());
        QCOMPARE(date.hasYear(), decoded.hasYear());
    }

    void testJsonDecodingLegacy() {
        // Old DB rows without dateType, proleptic2, year2, month2, day2 must decode as SINGLE
        const auto legacyJson =
            u"{\"dateModifier\":\"DURING\",\"dateQuality\":\"EXACT\",\"day\":true,\"month\":true,\"proleptic\":2452095,\"userText\":\"\",\"year\":true}"_s;
        auto decoded = GenealogicalDate::fromDatabaseRepresentation(legacyJson);
        QCOMPARE(decoded.type(), GenealogicalDate::SINGLE);
        QCOMPARE(decoded.modifier(), GenealogicalDate::DURING);
        QCOMPARE(decoded.prolepticRepresentation(), QDate(2001, 7, 4));
    }

    void testParseUserString() {
        auto userString = date.toDisplayText();
        auto otherDate = GenealogicalDate::fromDisplayText(userString);
        QCOMPARE(otherDate.toDatabaseRepresentation(), date.toDatabaseRepresentation());
    }

    void testDisplayStringWithoutAnything() {
        auto formattedDate = date.prolepticRepresentation().toString(u"yyyy-MM-dd"_s);
        auto result = GenealogicalDate::fromDisplayText(formattedDate);
        QCOMPARE(result.prolepticRepresentation(), date.prolepticRepresentation());
        QCOMPARE(result.modifier(), GenealogicalDate::NONE);
        QCOMPARE(result.quality(), GenealogicalDate::EXACT);
    }

    void testEquals() {
        QCOMPARE(date, date);
        QVERIFY(date == date);
        QVERIFY(!(date != date));
    }

    // Phase 1 regression: crash fixes
    void testFromDisplayTextSingleToken() {
        // "before" alone leaves empty remainder — must not crash
        auto result = GenealogicalDate::fromDisplayText(u"before"_s);
        QVERIFY(!result.isValid());
        QVERIFY(result.isNull());
    }

    void testFromDisplayTextEmpty() {
        auto result = GenealogicalDate::fromDisplayText(u""_s);
        QVERIFY(!result.isValid());
        QVERIFY(result.isNull());
    }

    // Phase 2: null / valid semantics
    void testDefaultConstructedIsNull() {
        GenealogicalDate d;
        QVERIFY(d.isNull());
        QVERIFY(!d.isValid());
        QVERIFY(!d.hasYear());
        QVERIFY(!d.hasMonth());
        QVERIFY(!d.hasDay());
    }

    // Phase 3: ABOUT modifier
    void testAboutModifierRoundtrip() {
        GenealogicalDate
            d(GenealogicalDate::ABOUT, GenealogicalDate::EXACT, QDate(1850, 7, 4), true, true, true, u""_s);
        QVERIFY(d.toDisplayText().startsWith(u"about"_s));
        auto reparsed = GenealogicalDate::fromDisplayText(d.toDisplayText());
        QCOMPARE(reparsed.modifier(), GenealogicalDate::ABOUT);
        QCOMPARE(reparsed.prolepticRepresentation(), QDate(1850, 7, 4));
    }

    // Phase 3: partial date display and parsing
    void testYearOnlyRoundtrip() {
        GenealogicalDate
            d(GenealogicalDate::NONE, GenealogicalDate::EXACT, QDate(1850, 1, 1), true, false, false, u""_s);
        QCOMPARE(d.toDisplayText(), u"1850"_s);
        auto reparsed = GenealogicalDate::fromDisplayText(u"1850"_s);
        QVERIFY(reparsed.hasYear());
        QVERIFY(!reparsed.hasMonth());
        QVERIFY(!reparsed.hasDay());
        QCOMPARE(reparsed.prolepticRepresentation().year(), 1850);
        QVERIFY(reparsed.isValid());
    }

    void testYearMonthRoundtrip() {
        GenealogicalDate
            d(GenealogicalDate::NONE, GenealogicalDate::EXACT, QDate(1850, 7, 1), true, true, false, u""_s);
        QCOMPARE(d.toDisplayText(), u"1850-07"_s);
        auto reparsed = GenealogicalDate::fromDisplayText(u"1850-07"_s);
        QVERIFY(reparsed.hasYear());
        QVERIFY(reparsed.hasMonth());
        QVERIFY(!reparsed.hasDay());
        QCOMPARE(reparsed.prolepticRepresentation(), QDate(1850, 7, 1));
        QVERIFY(reparsed.isValid());
    }

    void testBeforeYearOnly() {
        auto d = GenealogicalDate::fromDisplayText(u"before 1850"_s);
        QCOMPARE(d.modifier(), GenealogicalDate::BEFORE);
        QVERIFY(d.hasYear());
        QVERIFY(!d.hasMonth());
        QVERIFY(!d.hasDay());
        QVERIFY(d.isValid());
    }

    // Phase 4: range and span
    void testRangeRoundtrip() {
        auto d = GenealogicalDate::makeRange(
            GenealogicalDate::EXACT,
            QDate(1850, 1, 1),
            true,
            true,
            true,
            QDate(1900, 12, 31),
            true,
            true,
            true
        );
        QCOMPARE(d.type(), GenealogicalDate::RANGE);
        QVERIFY(d.isValid());
        const auto text = d.toDisplayText();
        QCOMPARE(text, u"between 1850-01-01 and 1900-12-31"_s);
        auto reparsed = GenealogicalDate::fromDisplayText(text);
        QCOMPARE(reparsed.type(), GenealogicalDate::RANGE);
        QCOMPARE(reparsed.prolepticRepresentation(), QDate(1850, 1, 1));
        QCOMPARE(reparsed.endPoint().proleptic, QDate(1900, 12, 31));
        QVERIFY(reparsed.isValid());
    }

    void testSpanRoundtrip() {
        auto d = GenealogicalDate::makeSpan(
            GenealogicalDate::EXACT,
            QDate(1850, 1, 1),
            true,
            true,
            true,
            QDate(1900, 12, 31),
            true,
            true,
            true
        );
        QCOMPARE(d.type(), GenealogicalDate::SPAN);
        QVERIFY(d.isValid());
        const auto text = d.toDisplayText();
        QCOMPARE(text, u"from 1850-01-01 to 1900-12-31"_s);
        auto reparsed = GenealogicalDate::fromDisplayText(text);
        QCOMPARE(reparsed.type(), GenealogicalDate::SPAN);
        QCOMPARE(reparsed.prolepticRepresentation(), QDate(1850, 1, 1));
        QCOMPARE(reparsed.endPoint().proleptic, QDate(1900, 12, 31));
        QVERIFY(reparsed.isValid());
    }

    void testRangeJsonRoundtrip() {
        auto d = GenealogicalDate::makeRange(
            GenealogicalDate::ESTIMATED,
            QDate(1850, 6, 1),
            true,
            true,
            false,
            QDate(1850, 9, 1),
            true,
            true,
            false
        );
        const auto reparsed = GenealogicalDate::fromDatabaseRepresentation(d.toDatabaseRepresentation());
        QCOMPARE(reparsed.type(), GenealogicalDate::RANGE);
        QCOMPARE(reparsed.quality(), GenealogicalDate::ESTIMATED);
        QCOMPARE(reparsed.prolepticRepresentation(), QDate(1850, 6, 1));
        QCOMPARE(reparsed.endPoint().proleptic, QDate(1850, 9, 1));
        QVERIFY(reparsed.hasMonth());
        QVERIFY(!reparsed.hasDay());
        QVERIFY(reparsed.endPoint().month);
        QVERIFY(!reparsed.endPoint().day);
    }

    void testTimeJsonRoundtrip() {
        GenealogicalDate d(GenealogicalDate::NONE, GenealogicalDate::EXACT, QDate(1950, 7, 4), true, true, true, u""_s);
        d.setStartTime(QTime(3, 47));
        const auto reparsed = GenealogicalDate::fromDatabaseRepresentation(d.toDatabaseRepresentation());
        QVERIFY(reparsed.startPoint().hasTime);
        QCOMPARE(reparsed.startPoint().wallTime, QTime(3, 47));
        QCOMPARE(reparsed.prolepticRepresentation(), QDate(1950, 7, 4));
    }

    void testTimeDisplayRoundtrip() {
        GenealogicalDate d(GenealogicalDate::NONE, GenealogicalDate::EXACT, QDate(1950, 7, 4), true, true, true, u""_s);
        d.setStartTime(QTime(3, 47));
        QCOMPARE(d.toDisplayText(), u"1950-07-04 03:47"_s);
        const auto reparsed = GenealogicalDate::fromDisplayText(d.toDisplayText());
        QVERIFY(reparsed.startPoint().hasTime);
        QCOMPARE(reparsed.startPoint().wallTime, QTime(3, 47));
        QCOMPARE(reparsed.prolepticRepresentation(), QDate(1950, 7, 4));
    }

    void testSortKeyIgnoresTime() {
        GenealogicalDate
            d1(GenealogicalDate::NONE, GenealogicalDate::EXACT, QDate(1950, 7, 4), true, true, true, u""_s);
        d1.setStartTime(QTime(3, 47));
        GenealogicalDate
            d2(GenealogicalDate::NONE, GenealogicalDate::EXACT, QDate(1950, 7, 4), true, true, true, u""_s);
        d2.setStartTime(QTime(23, 59));
        QCOMPARE(d1.sortKey(), d2.sortKey());
    }

    void testLegacyJsonNoTime() {
        const auto legacyJson =
            u"{\"dateModifier\":\"NONE\",\"dateQuality\":\"EXACT\",\"day\":true,\"month\":true,\"proleptic\":2433283,\"userText\":\"\",\"year\":true}"_s;
        const auto decoded = GenealogicalDate::fromDatabaseRepresentation(legacyJson);
        QVERIFY(!decoded.startPoint().hasTime);
        QVERIFY(!decoded.startPoint().wallTime.isValid());
    }
};

QTEST_MAIN(TestOpaDate)

#include "genealogical_date.moc"
