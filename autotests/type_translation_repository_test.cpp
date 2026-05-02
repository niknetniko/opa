/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
#include "../src/domain/event/event_type_translation_repository.h"
#include "../src/domain/location/location_type_translation_repository.h"
#include "./test_utils.h"
#include "database/database.h"
#include "database/schema.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestTypeTranslationRepositories : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void init() {
        QVERIFY(QSqlDatabase::isDriverAvailable(u"QSQLITE"_s));
        openDatabase(u":memory:"_s, false);
    }

    void cleanup() {
        QSqlDatabase::database().close();
    }

    void testEventTypeTranslation_insertAndFetch() {
        const auto typeId = insertQuery(u"INSERT INTO event_types (type, builtin) VALUES ('Custom', false)"_s);

        EventTypeTranslationRepository repo;
        auto id = repo.insert(typeId, u"nl"_s, u"Aangepast"_s);
        QVERIFY(id.has_value());

        auto all = repo.findAllForType(typeId);
        QCOMPARE(all.size(), 1);
        QCOMPARE(all.first().locale, u"nl"_s);
        QCOMPARE(all.first().name, u"Aangepast"_s);
        QCOMPARE(all.first().typeId, typeId);
    }

    void testEventTypeTranslation_remove() {
        const auto typeId = insertQuery(u"INSERT INTO event_types (type, builtin) VALUES ('Custom', false)"_s);

        EventTypeTranslationRepository repo;
        auto id = repo.insert(typeId, u"nl"_s, u"Aangepast"_s);
        QVERIFY(id.has_value());
        QVERIFY(repo.remove(*id));
        QCOMPARE(repo.findAllForType(typeId).size(), 0);
    }

    void testEventTypeTranslation_cascadeDelete() {
        const auto typeId = insertQuery(u"INSERT INTO event_types (type, builtin) VALUES ('Custom', false)"_s);

        EventTypeTranslationRepository repo;
        QVERIFY(repo.insert(typeId, u"nl"_s, u"Aangepast"_s).has_value());
        QCOMPARE(repo.findAllForType(typeId).size(), 1);

        QSqlQuery del;
        del.prepare(u"DELETE FROM event_types WHERE id = :id"_s);
        del.bindValue(u":id"_s, typeId);
        VERIFY_OR_THROW2(del.exec(), del);

        QCOMPARE(repo.findAllForType(typeId).size(), 0);
    }

    void testEventTypeTranslation_findByTypeStringAndLocale() {
        const auto typeId = insertQuery(u"INSERT INTO event_types (type, builtin) VALUES ('Adoptie', false)"_s);

        EventTypeTranslationRepository repo;
        QVERIFY(repo.insert(typeId, u"en"_s, u"Adoption"_s).has_value());

        auto result = repo.findByTypeStringAndLocale(u"Adoptie"_s, u"en"_s);
        QVERIFY(result.has_value());
        QCOMPARE(*result, u"Adoption"_s);

        auto missing = repo.findByTypeStringAndLocale(u"Adoptie"_s, u"fr"_s);
        QVERIFY(!missing.has_value());
    }

    void testLocationTypeTranslation_insertAndFetch() {
        const auto typeId = insertQuery(u"INSERT INTO location_types (type, builtin) VALUES ('Boerderij', false)"_s);

        LocationTypeTranslationRepository repo;
        auto id = repo.insert(typeId, u"en"_s, u"Farm"_s);
        QVERIFY(id.has_value());

        auto all = repo.findAllForType(typeId);
        QCOMPARE(all.size(), 1);
        QCOMPARE(all.first().name, u"Farm"_s);
    }

    void testLocationTypeTranslation_findByTypeStringAndLocale() {
        const auto typeId = insertQuery(u"INSERT INTO location_types (type, builtin) VALUES ('Boerderij', false)"_s);

        LocationTypeTranslationRepository repo;
        QVERIFY(repo.insert(typeId, u"en"_s, u"Farm"_s).has_value());

        auto result = repo.findByTypeStringAndLocale(u"Boerderij"_s, u"en"_s);
        QVERIFY(result.has_value());
        QCOMPARE(*result, u"Farm"_s);
    }
};

QTEST_MAIN(TestTypeTranslationRepositories)

#include "type_translation_repository_test.moc"
