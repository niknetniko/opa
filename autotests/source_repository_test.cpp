/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
// ReSharper disable CppDFAUnreachableFunctionCall
#include "../src/domain/source/source_repository.h"

#include "./test_utils.h"
#include "database/database.h"
#include "database/schema.h"

#include <QSqlDatabase>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestSourceRepository : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void init() {
        QVERIFY(QSqlDatabase::isDriverAvailable(u"QSQLITE"_s));
        openDatabase(u":memory:"_s, false);
    }

    void cleanup() {
        auto db = QSqlDatabase::database();
        db.close();
    }

    void testInsertAndFindById() {
        SourceRepository repo;
        auto sourceId = repo.insert(u"Test Source"_s, u"Book"_s, QString(), QString(), 3, QString(), std::nullopt);
        QVERIFY(sourceId.has_value());

        auto source = repo.findById(*sourceId);
        QVERIFY(source.has_value());
        QCOMPARE(source->id, *sourceId);
        QCOMPARE(source->confidence, 3);
        QCOMPARE(source->title, u"Test Source"_s);
        QCOMPARE(source->type, u"Book"_s);
        QVERIFY(!source->parentId.has_value());
    }

    void testFindAll() {
        SourceRepository repo;
        repo.insert(u"Source A"_s, u"Book"_s, QString(), QString(), 3, QString(), std::nullopt);
        repo.insert(u"Source B"_s, u"Article"_s, QString(), QString(), 4, QString(), std::nullopt);

        auto sources = repo.findAll();
        QCOMPARE(sources.size(), 2);
    }

    void testUpdate() {
        SourceRepository repo;
        auto sourceId = repo.insert(u"Original"_s, u"Book"_s, QString(), QString(), 3, QString(), std::nullopt);
        QVERIFY(sourceId.has_value());

        bool ok = repo.update(
            *sourceId,
            u"My Title"_s,
            u"Article"_s,
            u"Author"_s,
            u"Publisher"_s,
            4,
            u"A note"_s,
            std::nullopt
        );
        QVERIFY(ok);

        auto updated = repo.findById(*sourceId);
        QVERIFY(updated.has_value());
        QCOMPARE(updated->title, u"My Title"_s);
        QCOMPARE(updated->type, u"Article"_s);
        QCOMPARE(updated->author, u"Author"_s);
        QCOMPARE(updated->publication, u"Publisher"_s);
        QCOMPARE(updated->confidence, 4);
        QCOMPARE(updated->note, u"A note"_s);
        QVERIFY(!updated->parentId.has_value());
    }

    void testUpdateWithParent() {
        SourceRepository repo;
        auto parentId = repo.insert(u"Parent Source"_s, u"Book"_s, QString(), QString(), 3, QString(), std::nullopt);
        auto childId = repo.insert(u"Child Source"_s, u"Book"_s, QString(), QString(), 3, QString(), std::nullopt);
        QVERIFY(parentId.has_value());
        QVERIFY(childId.has_value());

        bool ok = repo.update(*childId, u"Child Source"_s, u"Book"_s, QString(), QString(), 3, QString(), parentId);
        QVERIFY(ok);

        auto updated = repo.findById(*childId);
        QVERIFY(updated.has_value());
        QVERIFY(updated->parentId.has_value());
        QCOMPARE(*updated->parentId, *parentId);
    }

    void testRemove() {
        SourceRepository repo;
        auto sourceId = repo.insert(u"To Remove"_s, u"Book"_s, QString(), QString(), 3, QString(), std::nullopt);
        QVERIFY(sourceId.has_value());

        bool ok = repo.remove(*sourceId);
        QVERIFY(ok);

        auto gone = repo.findById(*sourceId);
        QVERIFY(!gone.has_value());
    }

    void testFindByIdNotFound() {
        SourceRepository repo;
        auto result = repo.findById(9999);
        QVERIFY(!result.has_value());
    }
};

QTEST_MAIN(TestSourceRepository)

#include "source_repository_test.moc"
