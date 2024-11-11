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
        u"{\"dateModifier\":\"DURING\",\"dateQuality\":\"EXACT\",\"day\":true,\"month\":true,\"proleptic\":2452095,\"userText\":\"\",\"year\":true}"_s;

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

    void testParseUserString() {
        auto userString = date.toDisplayText();
        auto otherDate = GenealogicalDate::fromDisplayText(userString);
        QCOMPARE(otherDate.toDatabaseRepresentation(), date.toDatabaseRepresentation());
    }

    void testDisplayStringWithoutAnything() {
        const QLocale local;
        auto format = local.dateFormat();
        auto formattedDate = date.prolepticRepresentation().toString(format);
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
};

QTEST_MAIN(TestOpaDate)

#include "genealogical_date.moc"
