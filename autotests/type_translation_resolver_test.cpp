/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
#include "../src/utils/type_translation_resolver.h"

#include "../src/domain/event/event_type_translation_repository.h"
#include "./test_utils.h"
#include "database/database.h"
#include "database/schema.h"

#include <QSqlDatabase>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

static const auto identity = [](const QString& s) -> QString { return s; };
static const auto uppercaseTranslator = [](const QString& s) -> QString { return s.toUpper(); };

static TypeTranslationResolver makeResolver(std::function<QString(const QString&)> builtinTranslator) {
    return TypeTranslationResolver(
        [](IntegerPrimaryKey typeId, const QString& locale) {
            return EventTypeTranslationRepository().findByTypeIdAndLocale(typeId, locale);
        },
        std::move(builtinTranslator)
    );
}

class TestTypeTranslationResolver : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void init() {
        QVERIFY(QSqlDatabase::isDriverAvailable(u"QSQLITE"_s));
        openDatabase(u":memory:"_s, false);
    }

    void cleanup() {
        QSqlDatabase::database().close();
    }

    void testBuiltinType_usesTranslatorLambda() {
        const auto result = makeResolver(uppercaseTranslator).resolve(u"Birth"_s, true, -1, u"en"_s);
        QCOMPARE(result, u"BIRTH"_s);
    }

    void testUserType_noTranslation_returnsStoredName() {
        const auto typeId = insertQuery(u"INSERT INTO event_types (type, builtin) VALUES ('Adoptie', false)"_s);
        const auto result = makeResolver(identity).resolve(u"Adoptie"_s, false, typeId, u"en"_s);
        QCOMPARE(result, u"Adoptie"_s);
    }

    void testUserType_exactLocaleMatch() {
        const auto typeId = insertQuery(u"INSERT INTO event_types (type, builtin) VALUES ('Adoptie', false)"_s);
        insertQuery(
            u"INSERT INTO event_type_translations (type_id, locale, name) VALUES (%1, 'en', 'Adoption')"_s.arg(typeId)
        );
        const auto result = makeResolver(identity).resolve(u"Adoptie"_s, false, typeId, u"en"_s);
        QCOMPARE(result, u"Adoption"_s);
    }

    void testUserType_languageOnlyFallback() {
        const auto typeId = insertQuery(u"INSERT INTO event_types (type, builtin) VALUES ('Adoptie', false)"_s);
        insertQuery(
            u"INSERT INTO event_type_translations (type_id, locale, name) VALUES (%1, 'nl', 'Adoptie NL')"_s.arg(typeId)
        );
        // nl_BE -> nl fallback
        const auto result = makeResolver(identity).resolve(u"Adoptie"_s, false, typeId, u"nl_BE"_s);
        QCOMPARE(result, u"Adoptie NL"_s);
    }

    void testUserType_noMatchAtAll_returnsStoredName() {
        const auto typeId = insertQuery(u"INSERT INTO event_types (type, builtin) VALUES ('Adoptie', false)"_s);
        insertQuery(u"INSERT INTO event_type_translations (type_id, locale, name) VALUES (%1, 'fr', 'Adoption FR')"_s
                        .arg(typeId));
        const auto result = makeResolver(identity).resolve(u"Adoptie"_s, false, typeId, u"en"_s);
        QCOMPARE(result, u"Adoptie"_s);
    }
};

QTEST_MAIN(TestTypeTranslationResolver)

#include "type_translation_resolver_test.moc"
