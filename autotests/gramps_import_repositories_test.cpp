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

class TestGrampsImportRepositories : public QObject {
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

    void testRepositoryIsImportedAsSource() {
        const auto xml = grampsXml(
            u"  <repositories>\n"
            "    <repository handle=\"rp0001\" change=\"0\" id=\"R0001\">\n"
            "      <rname>National Archives</rname>\n"
            "      <type>Archive</type>\n"
            "    </repository>\n"
            "  </repositories>\n"_s
        );
        QVERIFY(runImport(validateGramps(xml)));

        auto q = execQuery(u"SELECT title, type FROM sources"_s);
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(), u"National Archives"_s);
        QCOMPARE(q.value(1).toString(), u"Archive"_s);
        QVERIFY(!q.next());
    }

    void testRepositoryExternalIdIsSet() {
        const auto xml = grampsXml(
            u"  <repositories>\n"
            "    <repository handle=\"rp0001\" change=\"0\" id=\"R0001\">\n"
            "      <rname>Library</rname>\n"
            "      <type>Library</type>\n"
            "    </repository>\n"
            "  </repositories>\n"_s
        );
        QVERIFY(runImport(validateGramps(xml)));

        auto q = execQuery(
            u"SELECT e.type, e.external_id FROM sources s JOIN source_external_ids e ON s.id = e.source_id"_s
        );
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(), u"gramps_id"_s);
        QCOMPARE(q.value(1).toString(), u"R0001"_s);
    }

    void testRepositoryNoteIsImported() {
        const auto xml = grampsXml(
            u"  <repositories>\n"
            "    <repository handle=\"rp0001\" change=\"0\" id=\"R0001\">\n"
            "      <rname>Library</rname>\n"
            "      <type>Library</type>\n"
            "      <noteref hlink=\"nt0001\"/>\n"
            "    </repository>\n"
            "  </repositories>\n"
            "  <notes>\n"
            "    <note handle=\"nt0001\" change=\"0\" id=\"N0001\" type=\"General\">\n"
            "      <text>Repository note</text>\n"
            "    </note>\n"
            "  </notes>\n"_s
        );
        QVERIFY(runImport(validateGramps(xml)));

        auto q = execQuery(u"SELECT note FROM sources"_s);
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(), u"Repository note"_s);
    }
};

QTEST_MAIN(TestGrampsImportRepositories)
#include "gramps_import_repositories_test.moc"
