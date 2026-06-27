/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
#include "../src/import/gramps_xml.h"

#include <QFuture>
#include <QTemporaryFile>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

namespace {

// Wraps body in a minimal valid Gramps XML document.
// URL is built at runtime to avoid moc misreading "//" in raw string literals.
QString grampsXml(const QString& body = {}) {
    const auto ns = u"http://gramps-project.org/xml/1.7.2/"_s;
    return u"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
           "<database xmlns=\""_s +
           ns +
           u"\">\n"
            "  <header>\n"
            "    <created date=\"2024-01-01\" version=\"5.1.3\"/>\n"
            "    <researcher/>\n"
            "  </header>\n"_s +
           body +
           u"</database>\n"_s;
}

GrampsXmlAnalysis runValidation(const QString& xml) {
    QTemporaryFile file;
    file.setAutoRemove(true);
    if (!file.open()) {
        qFatal("Could not create temporary file for test");
    }
    file.write(xml.toUtf8());
    file.flush();
    file.close();

    QPromise<GrampsXmlAnalysis> promise;
    promise.start();
    validateGrampsXml(promise, file.fileName());
    promise.finish();

    return promise.future().result();
}

} // namespace

class TestValidateGrampsXml : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testMinimalValidFileIsAccepted() {
        const auto result = runValidation(grampsXml());

        QVERIFY(result.valid);
        QCOMPARE(result.people, 0);
        QCOMPARE(result.families, 0);
        QCOMPARE(result.events, 0);
        QCOMPARE(result.sources, 0);
        QCOMPARE(result.places, 0);
    }

    void testEntityCountsAreCorrect() {
        const auto body =
            u"  <events>\n"
             "    <event handle=\"ee0001\" change=\"0\"><type>Birth</type></event>\n"
             "  </events>\n"
             "  <people>\n"
             "    <person handle=\"pp0001\" change=\"0\"><gender>M</gender></person>\n"
             "    <person handle=\"pp0002\" change=\"0\"><gender>F</gender></person>\n"
             "  </people>\n"
             "  <families>\n"
             "    <family handle=\"ff0001\" change=\"0\"/>\n"
             "  </families>\n"
             "  <sources>\n"
             "    <source handle=\"ss0001\" change=\"0\"/>\n"
             "    <source handle=\"ss0002\" change=\"0\"/>\n"
             "  </sources>\n"
             "  <places>\n"
             "    <placeobj handle=\"pl0001\" change=\"0\" type=\"\"><pname value=\"Somewhere\"/></placeobj>\n"
             "    <placeobj handle=\"pl0002\" change=\"0\" type=\"\"><pname value=\"Elsewhere\"/></placeobj>\n"
             "    <placeobj handle=\"pl0003\" change=\"0\" type=\"\"><pname value=\"Nowhere\"/></placeobj>\n"
             "  </places>\n"_s;

        const auto result = runValidation(grampsXml(body));

        QVERIFY(result.valid);
        QCOMPARE(result.people, 2);
        QCOMPARE(result.families, 1);
        QCOMPARE(result.events, 1);
        QCOMPARE(result.sources, 2);
        QCOMPARE(result.places, 3);
    }

    void testNonXmlFileIsRejected() {
        const auto result = runValidation(u"this is not xml"_s);

        QVERIFY(!result.valid);
        QVERIFY(!result.error.isEmpty());
    }

    void testWellFormedButWrongSchemaIsRejected() {
        const auto result = runValidation(u"<?xml version=\"1.0\"?><root><child/></root>"_s);

        QVERIFY(!result.valid);
        QVERIFY(!result.error.isEmpty());
    }

    void testRealFileIsAccepted() {
        const QString path = QFINDTESTDATA("example-1.7.2.gramps");
        QVERIFY2(!path.isEmpty(), "Could not find example-1.7.2.gramps");

        QPromise<GrampsXmlAnalysis> promise;
        promise.start();
        validateGrampsXml(promise, path);
        promise.finish();

        const auto result = promise.future().result();

        QVERIFY(result.valid);
        QCOMPARE(result.people, 2157);
        QCOMPARE(result.families, 762);
        QCOMPARE(result.events, 3432);
        QCOMPARE(result.sources, 4);
        QCOMPARE(result.places, 1296);
    }

    void testNonExistentFileIsRejected() {
        QPromise<GrampsXmlAnalysis> promise;
        promise.start();
        validateGrampsXml(promise, u"/tmp/this-file-does-not-exist-opa-test.xml"_s);
        promise.finish();

        const auto result = promise.future().result();

        QVERIFY(!result.valid);
        QVERIFY(!result.error.isEmpty());
    }
};

QTEST_MAIN(TestValidateGrampsXml)
#include "gramps_xml_test.moc"
