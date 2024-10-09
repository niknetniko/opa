//
// Created by niko on 10/10/24.
//

#include <QTest>
#include <QSqlDatabase>
#include "utils/opa_date.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

class TestOpaDate : public QObject {
Q_OBJECT

    const OpaDate date = OpaDate(
            OpaDate::DURING,
            OpaDate::EXACT,
            QDate(2001, 7, 4),
            QDate(),
            true,
            false,
            true,
            QStringLiteral("Hallo this is a test")
    );

    const QString json = QStringLiteral(
            "{\"dateModifier\":\"DURING\",\"dateQuality\":\"EXACT\",\"day\":true,\"month\":false,\"proleptic\":2452095,\"prolepticEnd\":-9223372036854775808,\"userText\":\"Hallo this is a test\",\"year\":true}");

private Q_SLOTS:
    void jsonEncoding() {
        QString generated = date.toDatabaseRepresentation();
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

#pragma clang diagnostic pop