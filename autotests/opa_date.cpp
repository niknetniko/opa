/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeConst
#include "utils/opa_date.h"

#include <QSqlDatabase>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestOpaDate : public QObject {
    Q_OBJECT

    const OpaDate date = OpaDate(
        OpaDate::DURING, OpaDate::EXACT, QDate(2001, 7, 4), QDate(), true, false, true, u"Hallo this is a test"_s
    );

    const QString json =
        u"{\"dateModifier\":\"DURING\",\"dateQuality\":\"EXACT\",\"day\":true,\"month\":false,\"proleptic\":2452095,\"userText\":\"Hallo this is a test\",\"year\":true}"_s;

private Q_SLOTS:
    void jsonEncoding() {
        const auto generated = date.toDatabaseRepresentation();
        QCOMPARE(generated, json);
    }

    void jsonDecoding() {
        auto decoded = OpaDate::fromDatabaseRepresentation(json);
        QCOMPARE(date.modifier(), decoded.modifier());
        QCOMPARE(date.quality(), decoded.quality());
        QCOMPARE(date.prolepticRepresentation(), decoded.prolepticRepresentation());
        QCOMPARE(date.prolepticRepresentationEnd(), decoded.prolepticRepresentationEnd());
        QCOMPARE(date.hasDay(), decoded.hasDay());
        QCOMPARE(date.hasMonth(), decoded.hasMonth());
        QCOMPARE(date.hasYear(), decoded.hasYear());
        QCOMPARE(date.text(), decoded.text());
    }
};

QTEST_MAIN(TestOpaDate)

#include "opa_date.moc"
