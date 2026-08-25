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

class TestGrampsImportMedia : public QObject {
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

    void testMediaIsImported() {
        const auto xml = grampsXml(
            u"  <objects>\n"
            "    <object handle=\"om0001\" change=\"0\" id=\"O0001\">\n"
            "      <file src=\"/photos/test.jpg\" mime=\"image/jpeg\" checksum=\"\" description=\"Test photo\"/>\n"
            "    </object>\n"
            "  </objects>\n"_s
        );
        QVERIFY(runImport(validateGramps(xml)));

        auto q = execQuery(u"SELECT path, mime_type, title FROM media"_s);
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(), u"/photos/test.jpg"_s);
        QCOMPARE(q.value(1).toString(), u"image/jpeg"_s);
        QCOMPARE(q.value(2).toString(), u"Test photo"_s);
        QVERIFY(!q.next());
    }

    void testMediaExternalIdIsSet() {
        const auto xml = grampsXml(
            u"  <objects>\n"
            "    <object handle=\"om0001\" change=\"0\" id=\"O0001\">\n"
            "      <file src=\"/photos/test.jpg\" mime=\"image/jpeg\" checksum=\"\" description=\"\"/>\n"
            "    </object>\n"
            "  </objects>\n"_s
        );
        QVERIFY(runImport(validateGramps(xml)));

        auto q =
            execQuery(u"SELECT e.type, e.external_id FROM media m JOIN media_external_ids e ON m.id = e.media_id"_s);
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(), u"gramps_id"_s);
        QCOMPARE(q.value(1).toString(), u"O0001"_s);
    }

    void testMediaMissingMimeTypeFallsBack() {
        const auto xml = grampsXml(
            u"  <objects>\n"
            "    <object handle=\"om0001\" change=\"0\" id=\"O0001\">\n"
            "      <file src=\"/file.bin\" mime=\"\" checksum=\"\" description=\"\"/>\n"
            "    </object>\n"
            "  </objects>\n"_s
        );
        QVERIFY(runImport(validateGramps(xml)));

        auto q = execQuery(u"SELECT mime_type FROM media"_s);
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(), u"application/octet-stream"_s);
    }
};

QTEST_MAIN(TestGrampsImportMedia)
#include "gramps_import_media_test.moc"
