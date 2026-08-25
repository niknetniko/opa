/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
#include "database/database.h"
#include "gramps_import_test_utils.h"

#include <QSqlDatabase>
#include <QTemporaryFile>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestGrampsImportLocations : public QObject {
    Q_OBJECT

    QString dbPath;

private Q_SLOTS:
    void init() {
        QVERIFY(QSqlDatabase::isDriverAvailable(u"QSQLITE"_s));
        QTemporaryFile f;
        QVERIFY(f.open());
        dbPath = f.fileName();
        f.setAutoRemove(false);
        f.close();
        QFile::remove(dbPath);
        openDatabase(dbPath, false);
    }

    void cleanup() {
        QSqlDatabase::database().close();
        QFile::remove(dbPath);
    }

    void testLocationIsImported() {
        const auto xml = grampsXml(
            u"  <places>\n"
            "    <placeobj handle=\"pl0001\" change=\"0\" id=\"P0001\" type=\"City\">\n"
            "      <pname value=\"Amsterdam\"/>\n"
            "    </placeobj>\n"
            "  </places>\n"_s
        );
        QVERIFY(runImport(validateGramps(xml)));

        auto q = execQuery(u"SELECT name, type FROM locations"_s);
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(), u"Amsterdam"_s);
        QCOMPARE(q.value(1).toString(), u"City"_s);
        QVERIFY(!q.next());
    }

    void testLocationExternalIdIsSet() {
        const auto xml = grampsXml(
            u"  <places>\n"
            "    <placeobj handle=\"pl0001\" change=\"0\" id=\"P0001\" type=\"\">\n"
            "      <pname value=\"Somewhere\"/>\n"
            "    </placeobj>\n"
            "  </places>\n"_s
        );
        QVERIFY(runImport(validateGramps(xml)));

        auto q = execQuery(
            u"SELECT e.type, e.external_id FROM locations l JOIN location_external_ids e ON l.id = e.location_id"_s
        );
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(), u"gramps_id"_s);
        QCOMPARE(q.value(1).toString(), u"P0001"_s);
    }

    void testLocationParentIsSet() {
        const auto xml = grampsXml(
            u"  <places>\n"
            "    <placeobj handle=\"pl0001\" change=\"0\" id=\"P0001\" type=\"Country\">\n"
            "      <pname value=\"Netherlands\"/>\n"
            "    </placeobj>\n"
            "    <placeobj handle=\"pl0002\" change=\"0\" id=\"P0002\" type=\"City\">\n"
            "      <pname value=\"Amsterdam\"/>\n"
            "      <placeref hlink=\"pl0001\"/>\n"
            "    </placeobj>\n"
            "  </places>\n"_s
        );
        QVERIFY(runImport(validateGramps(xml)));

        auto q = execQuery(
            u"SELECT l.name, p.name FROM locations l LEFT JOIN locations p ON l.parent_id = p.id WHERE l.name = 'Amsterdam'"_s
        );
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(), u"Amsterdam"_s);
        QCOMPARE(q.value(1).toString(), u"Netherlands"_s);
    }

    void testLocationHierarchyInReverseXmlOrder() {
        // Child appears before parent in XML — topological sort must handle this.
        const auto xml = grampsXml(
            u"  <places>\n"
            "    <placeobj handle=\"pl0002\" change=\"0\" id=\"P0002\" type=\"City\">\n"
            "      <pname value=\"Amsterdam\"/>\n"
            "      <placeref hlink=\"pl0001\"/>\n"
            "    </placeobj>\n"
            "    <placeobj handle=\"pl0001\" change=\"0\" id=\"P0001\" type=\"Country\">\n"
            "      <pname value=\"Netherlands\"/>\n"
            "    </placeobj>\n"
            "  </places>\n"_s
        );
        QVERIFY(runImport(validateGramps(xml)));

        auto q = execQuery(
            u"SELECT l.name, p.name FROM locations l LEFT JOIN locations p ON l.parent_id = p.id WHERE l.name = 'Amsterdam'"_s
        );
        QVERIFY(q.next());
        QCOMPARE(q.value(1).toString(), u"Netherlands"_s);
    }

    void testLocationCoordinatesAreSet() {
        const auto xml = grampsXml(
            u"  <places>\n"
            "    <placeobj handle=\"pl0001\" change=\"0\" id=\"P0001\" type=\"City\">\n"
            "      <pname value=\"Amsterdam\"/>\n"
            "      <coord long=\"4.9041\" lat=\"52.3676\"/>\n"
            "    </placeobj>\n"
            "  </places>\n"_s
        );
        QVERIFY(runImport(validateGramps(xml)));

        auto q = execQuery(u"SELECT latitude, longitude FROM locations"_s);
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toDouble(), 52.3676);
        QCOMPARE(q.value(1).toDouble(), 4.9041);
    }

    void testLocationCoordinatesAreNullWhenAbsent() {
        const auto xml = grampsXml(
            u"  <places>\n"
            "    <placeobj handle=\"pl0001\" change=\"0\" id=\"P0001\" type=\"City\">\n"
            "      <pname value=\"Amsterdam\"/>\n"
            "    </placeobj>\n"
            "  </places>\n"_s
        );
        QVERIFY(runImport(validateGramps(xml)));

        auto q = execQuery(u"SELECT latitude, longitude FROM locations"_s);
        QVERIFY(q.next());
        QVERIFY(q.value(0).isNull());
        QVERIFY(q.value(1).isNull());
    }

    void testLocationNoteIsSet() {
        const auto xml = grampsXml(
            u"  <places>\n"
            "    <placeobj handle=\"pl0001\" change=\"0\" id=\"P0001\" type=\"City\">\n"
            "      <pname value=\"Amsterdam\"/>\n"
            "      <noteref hlink=\"nt0001\"/>\n"
            "    </placeobj>\n"
            "  </places>\n"
            "  <notes>\n"
            "    <note handle=\"nt0001\" change=\"0\" id=\"N0001\" type=\"General\">\n"
            "      <text>Historic city</text>\n"
            "    </note>\n"
            "  </notes>\n"_s
        );
        QVERIFY(runImport(validateGramps(xml)));

        auto q = execQuery(u"SELECT note FROM locations"_s);
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(), u"Historic city"_s);
    }

    void testLocationMediaIsLinked() {
        const auto xml = grampsXml(
            u"  <places>\n"
            "    <placeobj handle=\"pl0001\" change=\"0\" id=\"P0001\" type=\"City\">\n"
            "      <pname value=\"Amsterdam\"/>\n"
            "      <objref hlink=\"om0001\"/>\n"
            "    </placeobj>\n"
            "  </places>\n"
            "  <objects>\n"
            "    <object handle=\"om0001\" change=\"0\" id=\"O0001\">\n"
            "      <file src=\"/photos/city.jpg\" mime=\"image/jpeg\" checksum=\"\" description=\"\"/>\n"
            "    </object>\n"
            "  </objects>\n"_s
        );
        QVERIFY(runImport(validateGramps(xml)));

        auto q = execQuery(u"SELECT COUNT(*) FROM location_media"_s);
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toInt(), 1);
    }
};

QTEST_MAIN(TestGrampsImportLocations)
#include "gramps_import_locations_test.moc"
