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
        auto sourceId =
            repo.insert(QString(), std::nullopt, QString(), QString(), u"Normal"_s, QString(), std::nullopt);
        QVERIFY(sourceId.has_value());

        auto source = repo.findById(*sourceId);
        QVERIFY(source.has_value());
        QCOMPARE(source->id, *sourceId);
        QCOMPARE(source->confidence, u"Normal"_s);
        QVERIFY(source->title.isEmpty());
        QVERIFY(!source->typeId.has_value());
        QVERIFY(!source->parentId.has_value());
    }

    void testFindAll() {
        SourceRepository repo;
        repo.insert(QString(), std::nullopt, QString(), QString(), u"Normal"_s, QString(), std::nullopt);
        repo.insert(QString(), std::nullopt, QString(), QString(), u"High"_s, QString(), std::nullopt);

        auto sources = repo.findAll();
        QCOMPARE(sources.size(), 2);
    }

    void testUpdate() {
        SourceRepository repo;
        auto sourceId =
            repo.insert(QString(), std::nullopt, QString(), QString(), u"Normal"_s, QString(), std::nullopt);
        QVERIFY(sourceId.has_value());

        bool ok = repo.update(
            *sourceId,
            u"My Title"_s,
            std::nullopt,
            u"Author"_s,
            u"Publisher"_s,
            u"High"_s,
            u"A note"_s,
            std::nullopt
        );
        QVERIFY(ok);

        auto updated = repo.findById(*sourceId);
        QVERIFY(updated.has_value());
        QCOMPARE(updated->title, u"My Title"_s);
        QVERIFY(!updated->typeId.has_value());
        QCOMPARE(updated->author, u"Author"_s);
        QCOMPARE(updated->publication, u"Publisher"_s);
        QCOMPARE(updated->confidence, u"High"_s);
        QCOMPARE(updated->note, u"A note"_s);
        QVERIFY(!updated->parentId.has_value());
    }

    void testUpdateWithTypeId() {
        SourceRepository repo;
        auto typeId = repo.insertSourceType(u"Book"_s);
        QVERIFY(typeId.has_value());

        auto sourceId =
            repo.insert(QString(), std::nullopt, QString(), QString(), u"Normal"_s, QString(), std::nullopt);
        QVERIFY(sourceId.has_value());

        bool ok = repo.update(*sourceId, QString(), typeId, QString(), QString(), u"Normal"_s, QString(), std::nullopt);
        QVERIFY(ok);

        auto updated = repo.findById(*sourceId);
        QVERIFY(updated.has_value());
        QVERIFY(updated->typeId.has_value());
        QCOMPARE(*updated->typeId, *typeId);
    }

    void testUpdateWithParent() {
        SourceRepository repo;
        auto parentId =
            repo.insert(QString(), std::nullopt, QString(), QString(), u"Normal"_s, QString(), std::nullopt);
        auto childId = repo.insert(QString(), std::nullopt, QString(), QString(), u"Normal"_s, QString(), std::nullopt);
        QVERIFY(parentId.has_value());
        QVERIFY(childId.has_value());

        bool ok =
            repo.update(*childId, QString(), std::nullopt, QString(), QString(), u"Normal"_s, QString(), parentId);
        QVERIFY(ok);

        auto updated = repo.findById(*childId);
        QVERIFY(updated.has_value());
        QVERIFY(updated->parentId.has_value());
        QCOMPARE(*updated->parentId, *parentId);
    }

    void testRemove() {
        SourceRepository repo;
        auto sourceId =
            repo.insert(QString(), std::nullopt, QString(), QString(), u"Normal"_s, QString(), std::nullopt);
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

    void testFindAllSourceTypes() {
        SourceRepository repo;
        auto id1 = repo.insertSourceType(u"Book"_s);
        auto id2 = repo.insertSourceType(u"Archive"_s);
        QVERIFY(id1.has_value());
        QVERIFY(id2.has_value());

        auto types = repo.findAllSourceTypes();
        QStringList typeStrings;
        for (const auto& e: types) {
            typeStrings.append(e.type);
        }
        QVERIFY(typeStrings.contains(u"Book"_s));
        QVERIFY(typeStrings.contains(u"Archive"_s));
    }

    void testIsSourceTypeUsed() {
        SourceRepository repo;
        auto typeId = repo.insertSourceType(u"Letter"_s);
        QVERIFY(typeId.has_value());

        QVERIFY(!repo.isSourceTypeUsed(*typeId));

        auto sourceId = repo.insert(QString(), typeId, QString(), QString(), u"Normal"_s, QString(), std::nullopt);
        QVERIFY(sourceId.has_value());
        QVERIFY(repo.isSourceTypeUsed(*typeId));
    }
};

QTEST_MAIN(TestSourceRepository)

#include "source_repository_test.moc"
