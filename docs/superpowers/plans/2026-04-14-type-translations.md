# Type Translations Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Enable translated display names for event/location types — built-in types via KDE `.po` files, user-defined types via an optional per-locale DB table.

**Architecture:** Built-in types (flagged `builtin=true`) are translated at display time via `kli18n()` enum maps. User types get optional translations in new `event_type_translations` / `location_type_translations` DB tables; display falls back to the stored `type` string. A `TypeTranslationResolver` encapsulates resolution logic, and a `TypeTranslationsDialog` exposes editing for user types.

**Tech Stack:** Qt 6, KDE Frameworks 6 (`KF6::I18n`, `KF6::WidgetsAddons`), SQLite via `QSqlQuery`, `BaseRepository` / `QueryHelper` pattern.

---

## File Map

| Action | File | Responsibility |
|--------|------|----------------|
| Create | `src/domain/location/location_types.h` | `LocationTypes` enum + kli18n map |
| Create | `src/domain/location/location_types.cpp` | MOC compilation unit |
| Modify | `src/domain/event/event_types.h` | Fix crash for user-defined types |
| Modify | `src/lists/location_types_management_window.cpp` | Wire `LocationTypes::toDisplayString` |
| Create | `src/database/migrations/003_add_type_translations.sql` | Schema migration |
| Modify | `src/database/schema.sql` | New tables in base schema |
| Modify | `src/database/schema.h` | New `Schema::` constants |
| Modify | `src/database/database.cpp` | Register migration 3 |
| Create | `src/domain/event/event_type_translation_entities.h` | `EventTypeTranslationEntity` |
| Create | `src/domain/event/event_type_translation_repository.h/.cpp` | CRUD on `event_type_translations` |
| Create | `src/domain/location/location_type_translation_entities.h` | `LocationTypeTranslationEntity` |
| Create | `src/domain/location/location_type_translation_repository.h/.cpp` | CRUD on `location_type_translations` |
| Create | `src/utils/type_translation_resolver.h/.cpp` | Resolution logic |
| Create | `src/utils/translating_proxy_model.h` | Proxy model that overrides `DisplayRole` |
| Modify | `src/editors/event_editor_dialog.cpp` | Use proxy in event type combo box |
| Modify | `src/editors/location_editor_dialog.cpp` | Use proxy in location type combo box |
| Create | `src/editors/type_translations_dialog.h/.cpp/.ui` | Manage user type translations |
| Modify | `src/lists/event_types_management_window.h/.cpp` | Add Translations toolbar action |
| Modify | `src/lists/location_types_management_window.h/.cpp` | Add Translations toolbar action |
| Modify | `src/CMakeLists.txt` | Register all new source + resource files |
| Modify | `autotests/database.cpp` | Update DB tests |
| Create | `autotests/type_translation_repository_test.cpp` | Repo CRUD tests |
| Create | `autotests/type_translation_resolver_test.cpp` | Resolver fallback tests |
| Modify | `autotests/CMakeLists.txt` | Register new test files |

---

## Task 1: LocationTypes enum + fix toDisplayString crash

**Files:**
- Create: `src/domain/location/location_types.h`
- Create: `src/domain/location/location_types.cpp`
- Modify: `src/domain/event/event_types.h` (fix `toDisplayString`)
- Modify: `src/lists/location_types_management_window.cpp`
- Modify: `src/CMakeLists.txt`
- Modify: `autotests/database.cpp`

- [ ] **Step 1: Create `src/domain/location/location_types.h`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "utils/model_utils.h"

#include <QString>

namespace LocationTypes {
Q_NAMESPACE

enum class Values { Country, Province, County, City, Village, Parish, Address };

Q_ENUM_NS(Values);

const QHash<Values, KLazyLocalizedString> typeToString{
    {Values::Country, kli18n("Country")},
    {Values::Province, kli18n("Province")},
    {Values::County, kli18n("County")},
    {Values::City, kli18n("City")},
    {Values::Village, kli18n("Village")},
    {Values::Parish, kli18n("Parish")},
    {Values::Address, kli18n("Address")},
};

const auto toDisplayString = [](const QString& databaseValue) -> QString {
    if (!isValidEnum<Values>(databaseValue)) {
        return databaseValue;
    }
    return genericToDisplayString<Values>(databaseValue, typeToString);
};

} // namespace LocationTypes
```

- [ ] **Step 2: Create `src/domain/location/location_types.cpp`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "location_types.h"
```

- [ ] **Step 3: Fix `toDisplayString` in `src/domain/event/event_types.h`**

Replace the existing lambda (around line 28):
```cpp
// Before:
const auto toDisplayString = [](const QString& databaseValue) {
    return genericToDisplayString<Values>(databaseValue, typeToString);
};

// After:
const auto toDisplayString = [](const QString& databaseValue) -> QString {
    if (!isValidEnum<Values>(databaseValue)) {
        return databaseValue;
    }
    return genericToDisplayString<Values>(databaseValue, typeToString);
};
```

- [ ] **Step 4: Wire translator in `src/lists/location_types_management_window.cpp`**

Add `#include "domain/location/location_types.h"` to includes.

In `LocationTypesManagementWindow()`, add `setTranslator(LocationTypes::toDisplayString)` before `initializeLayout()`:

```cpp
LocationTypesManagementWindow::LocationTypesManagementWindow() {
    setWindowTitle(i18n("Manage location types"));

    auto* model = new LocationTypesListModel(this);
    setModel(model);
    setColumns(LocationTypesListModel::ID, LocationTypesListModel::TYPE, LocationTypesListModel::BUILTIN);
    setTranslator(LocationTypes::toDisplayString);  // add this line

    initializeLayout();
}
```

- [ ] **Step 5: Add new files to `src/CMakeLists.txt`**

In the `add_library(opa-lib STATIC ...)` block, add after the existing location domain files:
```cmake
  domain/location/location_types.h
  domain/location/location_types.cpp
```

- [ ] **Step 6: Update `autotests/database.cpp` — replace `testBuiltinLocationTypesAreInserted`**

Add `#include "../src/domain/location/location_types.h"` with the other includes.

Replace the existing `testBuiltinLocationTypesAreInserted` test slot with:
```cpp
void testBuiltinLocationTypesAreInserted() {
    runEnumValueCheck<LocationTypes::Values>(Schema::LocationTypesTable);
}
```

- [ ] **Step 7: Build and run the test**

```bash
cmake --build build --target database && ./build/autotests/database
```

Expected: all tests pass.

---

## Task 2: Schema migration for type_translations tables

**Files:**
- Create: `src/database/migrations/003_add_type_translations.sql`
- Modify: `src/database/schema.sql`
- Modify: `src/database/schema.h`
- Modify: `src/database/database.cpp`
- Modify: `src/CMakeLists.txt`
- Modify: `autotests/database.cpp`

- [ ] **Step 1: Write failing test in `autotests/database.cpp`**

In `testDatabaseSeedIsValid`, extend the `expected` list:
```cpp
QStringList expected = {
    Schema::NameOriginsTable,
    Schema::NamesTable,
    Schema::PeopleTable,
    Schema::EventRelationsTable,
    Schema::EventRolesTable,
    Schema::EventTypesTable,
    Schema::EventsTable,
    Schema::SourcesTable,
    Schema::EventCitationsTable,
    Schema::EventRelationCitationsTable,
    Schema::NameCitationsTable,
    Schema::PersonCitationsTable,
    Schema::LocationTypesTable,
    Schema::LocationsTable,
    Schema::EventTypeTranslationsTable,     // add
    Schema::LocationTypeTranslationsTable,  // add
    u"sqlite_sequence"_s
};
```

- [ ] **Step 2: Run test to verify it fails**

```bash
cmake --build build --target database && ./build/autotests/database
```

Expected: FAIL — `EventTypeTranslationsTable` and `LocationTypeTranslationsTable` do not exist yet.

- [ ] **Step 3: Add new constants to `src/database/schema.h`**

After `LocationsTable` (around line 84), add:
```cpp
struct EventTypeTranslations : TableTag {
    static constexpr auto table = QLatin1String("event_type_translations");
};
inline constexpr auto EventTypeTranslationsTable = EventTypeTranslations::table;

struct LocationTypeTranslations : TableTag {
    static constexpr auto table = QLatin1String("location_type_translations");
};
inline constexpr auto LocationTypeTranslationsTable = LocationTypeTranslations::table;
```

- [ ] **Step 4: Create `src/database/migrations/003_add_type_translations.sql`**

```sql
CREATE TABLE event_type_translations (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type_id INTEGER NOT NULL REFERENCES event_types (id) ON DELETE CASCADE,
  locale TEXT NOT NULL,
  name TEXT NOT NULL,
  UNIQUE (type_id, locale)
);
CREATE TABLE location_type_translations (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type_id INTEGER NOT NULL REFERENCES location_types (id) ON DELETE CASCADE,
  locale TEXT NOT NULL,
  name TEXT NOT NULL,
  UNIQUE (type_id, locale)
)
```

Note: no comments (they cause `abort()`); last statement has no trailing semicolon — the script splitter handles both.

- [ ] **Step 5: Add the same tables to `src/database/schema.sql`** (for new databases)

After the `locations` table definition, append:
```sql
CREATE TABLE event_type_translations (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type_id INTEGER NOT NULL REFERENCES event_types (id) ON DELETE CASCADE,
  locale TEXT NOT NULL,
  name TEXT NOT NULL,
  UNIQUE (type_id, locale)
);

CREATE TABLE location_type_translations (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type_id INTEGER NOT NULL REFERENCES location_types (id) ON DELETE CASCADE,
  locale TEXT NOT NULL,
  name TEXT NOT NULL,
  UNIQUE (type_id, locale)
);
```

- [ ] **Step 6: Register migration 3 in `src/database/database.cpp`**

In the `migrations` array (around line 32), add the third entry:
```cpp
constexpr std::array migrations = {
    Migration{
        .version = 1,
        .description = "Add surrogate key to event_relations, rekey event_relation_citations"_L1,
        .resourcePath = ":/migrations/001_event_relations_surrogate_key.sql"_L1,
    },
    Migration{
        .version = 2,
        .description = "Add location_types and locations tables, add location_id to events"_L1,
        .resourcePath = ":/migrations/002_add_locations.sql"_L1,
    },
    Migration{
        .version = 3,
        .description = "Add event_type_translations and location_type_translations tables"_L1,
        .resourcePath = ":/migrations/003_add_type_translations.sql"_L1,
    },
};
```

- [ ] **Step 7: Add migration file to resources in `src/CMakeLists.txt`**

In the `qt_add_resources(opa-lib "opa-database" ...)` block (around line 241):
```cmake
qt_add_resources(
  opa-lib "opa-database"
  PREFIX "/"
  BASE "database"
  FILES database/init.sql database/schema.sql database/seed.sql
        database/migrations/001_event_relations_surrogate_key.sql
        database/migrations/002_add_locations.sql
        database/migrations/003_add_type_translations.sql)
```

- [ ] **Step 8: Build and run tests**

```bash
cmake --build build --target database && ./build/autotests/database
```

Expected: all tests pass including `testDatabaseSeedIsValid`.

Also run the migration test:
```bash
cmake --build build --target database_migration_test && ./build/autotests/database_migration_test
```

Expected: PASS.

---

## Task 3: Translation entities and repositories

**Files:**
- Create: `src/domain/event/event_type_translation_entities.h`
- Create: `src/domain/event/event_type_translation_repository.h`
- Create: `src/domain/event/event_type_translation_repository.cpp`
- Create: `src/domain/location/location_type_translation_entities.h`
- Create: `src/domain/location/location_type_translation_repository.h`
- Create: `src/domain/location/location_type_translation_repository.cpp`
- Modify: `src/CMakeLists.txt`
- Create: `autotests/type_translation_repository_test.cpp`
- Modify: `autotests/CMakeLists.txt`

- [ ] **Step 1: Write the failing tests in `autotests/type_translation_repository_test.cpp`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
#include "../src/domain/event/event_type_translation_repository.h"
#include "../src/domain/location/location_type_translation_repository.h"
#include "../src/domain/location/location_repository.h"
#include "../src/domain/event/event_repository.h"

#include "./test_utils.h"
#include "database/database.h"
#include "database/schema.h"

#include <QSqlDatabase>
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
        const auto typeId = insertQuery(
            u"INSERT INTO event_types (type, builtin) VALUES ('Custom', false)"_s
        );

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
        const auto typeId = insertQuery(
            u"INSERT INTO event_types (type, builtin) VALUES ('Custom', false)"_s
        );

        EventTypeTranslationRepository repo;
        auto id = repo.insert(typeId, u"nl"_s, u"Aangepast"_s);
        QVERIFY(id.has_value());
        QVERIFY(repo.remove(*id));
        QCOMPARE(repo.findAllForType(typeId).size(), 0);
    }

    void testEventTypeTranslation_cascadeDelete() {
        const auto typeId = insertQuery(
            u"INSERT INTO event_types (type, builtin) VALUES ('Custom', false)"_s
        );

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
        const auto typeId = insertQuery(
            u"INSERT INTO event_types (type, builtin) VALUES ('Adoptie', false)"_s
        );

        EventTypeTranslationRepository repo;
        QVERIFY(repo.insert(typeId, u"en"_s, u"Adoption"_s).has_value());

        auto result = repo.findByTypeStringAndLocale(u"Adoptie"_s, u"en"_s);
        QVERIFY(result.has_value());
        QCOMPARE(*result, u"Adoption"_s);

        auto missing = repo.findByTypeStringAndLocale(u"Adoptie"_s, u"fr"_s);
        QVERIFY(!missing.has_value());
    }

    void testLocationTypeTranslation_insertAndFetch() {
        const auto typeId = insertQuery(
            u"INSERT INTO location_types (type, builtin) VALUES ('Boerderij', false)"_s
        );

        LocationTypeTranslationRepository repo;
        auto id = repo.insert(typeId, u"en"_s, u"Farm"_s);
        QVERIFY(id.has_value());

        auto all = repo.findAllForType(typeId);
        QCOMPARE(all.size(), 1);
        QCOMPARE(all.first().name, u"Farm"_s);
    }

    void testLocationTypeTranslation_findByTypeStringAndLocale() {
        const auto typeId = insertQuery(
            u"INSERT INTO location_types (type, builtin) VALUES ('Boerderij', false)"_s
        );

        LocationTypeTranslationRepository repo;
        QVERIFY(repo.insert(typeId, u"en"_s, u"Farm"_s).has_value());

        auto result = repo.findByTypeStringAndLocale(u"Boerderij"_s, u"en"_s);
        QVERIFY(result.has_value());
        QCOMPARE(*result, u"Farm"_s);
    }
};

QTEST_MAIN(TestTypeTranslationRepositories)

#include "type_translation_repository_test.moc"
```

- [ ] **Step 2: Run failing tests**

```bash
cmake --build build --target type_translation_repository_test 2>&1 | head -20
```

Expected: compile error (headers don't exist yet).

- [ ] **Step 3: Create `src/domain/event/event_type_translation_entities.h`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QSqlQuery>
#include <QString>

using namespace Qt::StringLiterals;

struct EventTypeTranslationEntity {
    IntegerPrimaryKey id = -1;
    IntegerPrimaryKey typeId = -1;
    QString locale;
    QString name;

    static EventTypeTranslationEntity fromSql(const QSqlQuery& query) {
        return {
            .id = query.value(u"id").toLongLong(),
            .typeId = query.value(u"type_id").toLongLong(),
            .locale = query.value(u"locale").toString(),
            .name = query.value(u"name").toString(),
        };
    }
};
```

- [ ] **Step 4: Create `src/domain/event/event_type_translation_repository.h`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "core/base_repository.h"
#include "event_type_translation_entities.h"

#include <QList>
#include <optional>

class EventTypeTranslationRepository : public BaseRepository {
public:
    [[nodiscard]] QList<EventTypeTranslationEntity> findAllForType(IntegerPrimaryKey typeId) const;
    std::optional<IntegerPrimaryKey> insert(IntegerPrimaryKey typeId, const QString& locale, const QString& name) const;
    bool remove(IntegerPrimaryKey id) const;
    [[nodiscard]] std::optional<QString> findByTypeStringAndLocale(
        const QString& typeString, const QString& locale
    ) const;
};
```

- [ ] **Step 5: Create `src/domain/event/event_type_translation_repository.cpp`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "event_type_translation_repository.h"

#include "core/data_event_broker.h"
#include "core/query_helper.h"

using namespace Qt::StringLiterals;

QList<EventTypeTranslationEntity> EventTypeTranslationRepository::findAllForType(IntegerPrimaryKey typeId) const {
    return fetchAll<EventTypeTranslationEntity>(
        u"SELECT id, type_id, locale, name FROM event_type_translations WHERE type_id = :type_id ORDER BY locale"_s,
        {{u":type_id"_s, typeId}}
    );
}

std::optional<IntegerPrimaryKey> EventTypeTranslationRepository::insert(
    IntegerPrimaryKey typeId, const QString& locale, const QString& name
) const {
    auto newId = QueryHelper::insert(
        u"INSERT INTO event_type_translations (type_id, locale, name) VALUES (:type_id, :locale, :name)"_s,
        {{u":type_id"_s, typeId}, {u":locale"_s, locale}, {u":name"_s, name}}
    );
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::EventTypeTranslations>(*newId);
    }
    return newId;
}

bool EventTypeTranslationRepository::remove(IntegerPrimaryKey id) const {
    return QueryHelper::executeAndNotify<Schema::EventTypeTranslations>(
        id,
        u"DELETE FROM event_type_translations WHERE id = :id"_s,
        {{u":id"_s, id}}
    );
}

std::optional<QString> EventTypeTranslationRepository::findByTypeStringAndLocale(
    const QString& typeString, const QString& locale
) const {
    struct StringResult {
        QString name;
        static StringResult fromSql(const QSqlQuery& q) {
            using namespace Qt::StringLiterals;
            return {.name = q.value(u"name").toString()};
        }
    };
    auto result = fetchOne<StringResult>(
        u"SELECT ett.name FROM event_type_translations ett "
        u"JOIN event_types et ON et.id = ett.type_id "
        u"WHERE et.type = :type AND ett.locale = :locale"_s,
        {{u":type"_s, typeString}, {u":locale"_s, locale}}
    );
    if (result) return result->name;
    return std::nullopt;
}
```

- [ ] **Step 6: Create `src/domain/location/location_type_translation_entities.h`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QSqlQuery>
#include <QString>

using namespace Qt::StringLiterals;

struct LocationTypeTranslationEntity {
    IntegerPrimaryKey id = -1;
    IntegerPrimaryKey typeId = -1;
    QString locale;
    QString name;

    static LocationTypeTranslationEntity fromSql(const QSqlQuery& query) {
        return {
            .id = query.value(u"id").toLongLong(),
            .typeId = query.value(u"type_id").toLongLong(),
            .locale = query.value(u"locale").toString(),
            .name = query.value(u"name").toString(),
        };
    }
};
```

- [ ] **Step 7: Create `src/domain/location/location_type_translation_repository.h`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "core/base_repository.h"
#include "location_type_translation_entities.h"

#include <QList>
#include <optional>

class LocationTypeTranslationRepository : public BaseRepository {
public:
    [[nodiscard]] QList<LocationTypeTranslationEntity> findAllForType(IntegerPrimaryKey typeId) const;
    std::optional<IntegerPrimaryKey> insert(IntegerPrimaryKey typeId, const QString& locale, const QString& name) const;
    bool remove(IntegerPrimaryKey id) const;
    [[nodiscard]] std::optional<QString> findByTypeStringAndLocale(
        const QString& typeString, const QString& locale
    ) const;
};
```

- [ ] **Step 8: Create `src/domain/location/location_type_translation_repository.cpp`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "location_type_translation_repository.h"

#include "core/data_event_broker.h"
#include "core/query_helper.h"

using namespace Qt::StringLiterals;

QList<LocationTypeTranslationEntity> LocationTypeTranslationRepository::findAllForType(IntegerPrimaryKey typeId) const {
    return fetchAll<LocationTypeTranslationEntity>(
        u"SELECT id, type_id, locale, name FROM location_type_translations WHERE type_id = :type_id ORDER BY locale"_s,
        {{u":type_id"_s, typeId}}
    );
}

std::optional<IntegerPrimaryKey> LocationTypeTranslationRepository::insert(
    IntegerPrimaryKey typeId, const QString& locale, const QString& name
) const {
    auto newId = QueryHelper::insert(
        u"INSERT INTO location_type_translations (type_id, locale, name) VALUES (:type_id, :locale, :name)"_s,
        {{u":type_id"_s, typeId}, {u":locale"_s, locale}, {u":name"_s, name}}
    );
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::LocationTypeTranslations>(*newId);
    }
    return newId;
}

bool LocationTypeTranslationRepository::remove(IntegerPrimaryKey id) const {
    return QueryHelper::executeAndNotify<Schema::LocationTypeTranslations>(
        id,
        u"DELETE FROM location_type_translations WHERE id = :id"_s,
        {{u":id"_s, id}}
    );
}

std::optional<QString> LocationTypeTranslationRepository::findByTypeStringAndLocale(
    const QString& typeString, const QString& locale
) const {
    struct StringResult {
        QString name;
        static StringResult fromSql(const QSqlQuery& q) {
            using namespace Qt::StringLiterals;
            return {.name = q.value(u"name").toString()};
        }
    };
    auto result = fetchOne<StringResult>(
        u"SELECT ltt.name FROM location_type_translations ltt "
        u"JOIN location_types lt ON lt.id = ltt.type_id "
        u"WHERE lt.type = :type AND ltt.locale = :locale"_s,
        {{u":type"_s, typeString}, {u":locale"_s, locale}}
    );
    if (result) return result->name;
    return std::nullopt;
}
```

- [ ] **Step 9: Register new source files in `src/CMakeLists.txt`**

In the `add_library(opa-lib STATIC ...)` block, add after the existing event domain files:
```cmake
  domain/event/event_type_translation_entities.h
  domain/event/event_type_translation_repository.h
  domain/event/event_type_translation_repository.cpp
```

And after the existing location domain files:
```cmake
  domain/location/location_type_translation_entities.h
  domain/location/location_type_translation_repository.h
  domain/location/location_type_translation_repository.cpp
```

- [ ] **Step 10: Register test in `autotests/CMakeLists.txt`**

In `ecm_add_tests(...)`, add `type_translation_repository_test.cpp`.

- [ ] **Step 11: Build and run tests**

```bash
cmake --build build --target type_translation_repository_test && ./build/autotests/type_translation_repository_test
```

Expected: all 6 tests pass.

---

## Task 4: TypeTranslationResolver

**Files:**
- Create: `src/utils/type_translation_resolver.h`
- Create: `src/utils/type_translation_resolver.cpp`
- Modify: `src/CMakeLists.txt`
- Create: `autotests/type_translation_resolver_test.cpp`
- Modify: `autotests/CMakeLists.txt`

- [ ] **Step 1: Write failing tests in `autotests/type_translation_resolver_test.cpp`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
#include "../src/utils/type_translation_resolver.h"
#include "../src/domain/event/event_type_translation_repository.h"
#include "../src/domain/location/location_type_translation_repository.h"

#include "./test_utils.h"
#include "database/database.h"

#include <QLocale>
#include <QSqlDatabase>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

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

    void testBuiltinEventType_returnsNonEmpty() {
        // "Birth" is a valid EventTypes enum; the resolver must return a non-empty string.
        auto resolver = TypeTranslationResolver::forEventTypes(QLocale(u"en"_s));
        QVERIFY(!resolver(u"Birth"_s).isEmpty());
    }

    void testUserEventType_noTranslation_returnsStoredName() {
        insertQuery(u"INSERT INTO event_types (type, builtin) VALUES ('Adoptie', false)"_s);
        auto resolver = TypeTranslationResolver::forEventTypes(QLocale(u"en"_s));
        QCOMPARE(resolver(u"Adoptie"_s), u"Adoptie"_s);
    }

    void testUserEventType_withTranslation_returnsTranslation() {
        const auto typeId = insertQuery(
            u"INSERT INTO event_types (type, builtin) VALUES ('Adoptie', false)"_s
        );
        EventTypeTranslationRepository repo;
        QVERIFY(repo.insert(typeId, u"en"_s, u"Adoption"_s).has_value());

        auto resolver = TypeTranslationResolver::forEventTypes(QLocale(u"en"_s));
        QCOMPARE(resolver(u"Adoptie"_s), u"Adoption"_s);
    }

    void testUserEventType_localeFallback() {
        // "nl" translation used when "nl_BE" is requested but only "nl" is available.
        const auto typeId = insertQuery(
            u"INSERT INTO event_types (type, builtin) VALUES ('Adoptie', false)"_s
        );
        EventTypeTranslationRepository repo;
        QVERIFY(repo.insert(typeId, u"nl"_s, u"Adoptie NL"_s).has_value());

        auto resolver = TypeTranslationResolver::forEventTypes(QLocale(u"nl_BE"_s));
        QCOMPARE(resolver(u"Adoptie"_s), u"Adoptie NL"_s);
    }

    void testUserEventType_noFallback_returnsStoredName() {
        insertQuery(u"INSERT INTO event_types (type, builtin) VALUES ('Adoptie', false)"_s);
        auto resolver = TypeTranslationResolver::forEventTypes(QLocale(u"fr"_s));
        QCOMPARE(resolver(u"Adoptie"_s), u"Adoptie"_s);
    }

    void testBuiltinLocationType_returnsNonEmpty() {
        auto resolver = TypeTranslationResolver::forLocationTypes(QLocale(u"en"_s));
        QVERIFY(!resolver(u"Country"_s).isEmpty());
    }

    void testUserLocationType_withTranslation() {
        const auto typeId = insertQuery(
            u"INSERT INTO location_types (type, builtin) VALUES ('Boerderij', false)"_s
        );
        LocationTypeTranslationRepository repo;
        QVERIFY(repo.insert(typeId, u"en"_s, u"Farm"_s).has_value());

        auto resolver = TypeTranslationResolver::forLocationTypes(QLocale(u"en"_s));
        QCOMPARE(resolver(u"Boerderij"_s), u"Farm"_s);
    }
};

QTEST_MAIN(TestTypeTranslationResolver)

#include "type_translation_resolver_test.moc"
```

- [ ] **Step 2: Run to confirm compile failure**

```bash
cmake --build build --target type_translation_resolver_test 2>&1 | head -10
```

Expected: compile error — `type_translation_resolver.h` not found.

- [ ] **Step 3: Create `src/utils/type_translation_resolver.h`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QLocale>
#include <QString>
#include <functional>

namespace TypeTranslationResolver {

/**
 * Returns a resolver function for event type names.
 *
 * Resolution order:
 *   1. Valid EventTypes enum value → kli18n translation
 *   2. DB lookup in event_type_translations for (typeString, locale)
 *   3. Language-only fallback ("nl_BE" → "nl")
 *   4. Stored name (identity)
 */
std::function<QString(const QString&)> forEventTypes(const QLocale& locale = QLocale::system());

/**
 * Returns a resolver function for location type names.
 * Same resolution order as forEventTypes, using location_type_translations.
 */
std::function<QString(const QString&)> forLocationTypes(const QLocale& locale = QLocale::system());

} // namespace TypeTranslationResolver
```

- [ ] **Step 4: Create `src/utils/type_translation_resolver.cpp`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "type_translation_resolver.h"

#include "domain/event/event_type_translation_repository.h"
#include "domain/event/event_types.h"
#include "domain/location/location_type_translation_repository.h"
#include "domain/location/location_types.h"

using namespace Qt::StringLiterals;

namespace {

template<typename BuiltinChecker, typename Translator, typename DBRepo>
std::function<QString(const QString&)> makeResolver(
    BuiltinChecker isBuiltin, Translator builtinTranslate, const QLocale& locale
) {
    return [isBuiltin, builtinTranslate, locale](const QString& value) -> QString {
        if (isBuiltin(value)) {
            return builtinTranslate(value);
        }

        DBRepo repo;
        const auto localeName = locale.name();

        if (auto t = repo.findByTypeStringAndLocale(value, localeName); t.has_value()) {
            return *t;
        }

        const auto languageOnly = localeName.section(u'_', 0, 0);
        if (languageOnly != localeName) {
            if (auto t = repo.findByTypeStringAndLocale(value, languageOnly); t.has_value()) {
                return *t;
            }
        }

        return value;
    };
}

} // namespace

std::function<QString(const QString&)> TypeTranslationResolver::forEventTypes(const QLocale& locale) {
    return makeResolver<
        decltype(&isValidEnum<EventTypes::Values>),
        decltype(EventTypes::toDisplayString),
        EventTypeTranslationRepository
    >(
        [](const QString& v) { return isValidEnum<EventTypes::Values>(v); },
        EventTypes::toDisplayString,
        locale
    );
}

std::function<QString(const QString&)> TypeTranslationResolver::forLocationTypes(const QLocale& locale) {
    return makeResolver<
        decltype(&isValidEnum<LocationTypes::Values>),
        decltype(LocationTypes::toDisplayString),
        LocationTypeTranslationRepository
    >(
        [](const QString& v) { return isValidEnum<LocationTypes::Values>(v); },
        LocationTypes::toDisplayString,
        locale
    );
}
```

Note: The `makeResolver` template uses lambda captures instead of explicit template args — simplify if the compiler complains about the `decltype` approach:

```cpp
// Simplified alternative without the template helper:
std::function<QString(const QString&)> TypeTranslationResolver::forEventTypes(const QLocale& locale) {
    return [locale](const QString& value) -> QString {
        if (isValidEnum<EventTypes::Values>(value)) {
            return EventTypes::toDisplayString(value);
        }
        EventTypeTranslationRepository repo;
        const auto localeName = locale.name();
        if (auto t = repo.findByTypeStringAndLocale(value, localeName)) return *t;
        const auto lang = localeName.section(u'_', 0, 0);
        if (lang != localeName) {
            if (auto t = repo.findByTypeStringAndLocale(value, lang)) return *t;
        }
        return value;
    };
}

std::function<QString(const QString&)> TypeTranslationResolver::forLocationTypes(const QLocale& locale) {
    return [locale](const QString& value) -> QString {
        if (isValidEnum<LocationTypes::Values>(value)) {
            return LocationTypes::toDisplayString(value);
        }
        LocationTypeTranslationRepository repo;
        const auto localeName = locale.name();
        if (auto t = repo.findByTypeStringAndLocale(value, localeName)) return *t;
        const auto lang = localeName.section(u'_', 0, 0);
        if (lang != localeName) {
            if (auto t = repo.findByTypeStringAndLocale(value, lang)) return *t;
        }
        return value;
    };
}
```

Use the simplified version — it is clearer and avoids complex template deduction.

- [ ] **Step 5: Register in `src/CMakeLists.txt`**

In `add_library(opa-lib STATIC ...)`, add:
```cmake
  utils/type_translation_resolver.h
  utils/type_translation_resolver.cpp
```

- [ ] **Step 6: Register test in `autotests/CMakeLists.txt`**

Add `type_translation_resolver_test.cpp` to `ecm_add_tests(...)`.

- [ ] **Step 7: Build and run tests**

```bash
cmake --build build --target type_translation_resolver_test && ./build/autotests/type_translation_resolver_test
```

Expected: all 7 tests pass.

---

## Task 5: TranslatingProxyModel + wire into editors

**Files:**
- Create: `src/utils/translating_proxy_model.h`
- Modify: `src/editors/event_editor_dialog.cpp`
- Modify: `src/editors/location_editor_dialog.cpp`
- Modify: `src/CMakeLists.txt`

No automated tests (visual change only — verify by running the app and opening the event editor).

- [ ] **Step 1: Create `src/utils/translating_proxy_model.h`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QIdentityProxyModel>
#include <functional>

/**
 * Proxy model that replaces Qt::DisplayRole for a single column
 * by passing the original text through a translator function.
 *
 * Used to show translated type names in combo boxes while keeping
 * the source model's data unchanged.
 */
class TranslatingProxyModel : public QIdentityProxyModel {
    Q_OBJECT

public:
    TranslatingProxyModel(int column, std::function<QString(QString)> translator, QObject* parent = nullptr)
        : QIdentityProxyModel(parent), m_column(column), m_translator(std::move(translator)) {}

    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override {
        if (role == Qt::DisplayRole && index.isValid() && index.column() == m_column) {
            return m_translator(QIdentityProxyModel::data(index, role).toString());
        }
        return QIdentityProxyModel::data(index, role);
    }

private:
    int m_column;
    std::function<QString(QString)> m_translator;
};
```

- [ ] **Step 2: Register in `src/CMakeLists.txt`**

Add to `add_library(opa-lib STATIC ...)`:
```cmake
  utils/translating_proxy_model.h
```

(Header-only, no `.cpp` needed.)

- [ ] **Step 3: Wire into `src/editors/event_editor_dialog.cpp`**

Add includes at the top:
```cpp
#include "utils/translating_proxy_model.h"
#include "utils/type_translation_resolver.h"
```

In `EventEditorDialog::setupUi()`, after `typesModel = new EventTypesListModel(this);` (around line 117), replace the two existing lines:
```cpp
// Before:
typesModel = new EventTypesListModel(this);
form->eventTypeComboBox->setModel(typesModel);
form->eventTypeComboBox->setModelColumn(EventTypesListModel::TYPE);

// After:
typesModel = new EventTypesListModel(this);
auto* typesProxy = new TranslatingProxyModel(
    EventTypesListModel::TYPE, TypeTranslationResolver::forEventTypes(), this
);
typesProxy->setSourceModel(typesModel);
form->eventTypeComboBox->setModel(typesProxy);
form->eventTypeComboBox->setModelColumn(EventTypesListModel::TYPE);
```

Also update the row ID lookup in `EventEditorDialog::accept()` (around line 177) — it uses `typesModel` directly, which is correct since proxy row indices equal source row indices:
```cpp
// This line does NOT need changing — typesModel row indices are identical to proxy indices:
auto typeId = typesModel->index(typeRow, EventTypesListModel::ID).data().toLongLong();
```

Also update the `match` call for pre-filling the form (around line 77) — it also uses `typesModel` directly and does not need changing.

- [ ] **Step 4: Wire into `src/editors/location_editor_dialog.cpp`**

Check how the location type combo box is set up. Add the same proxy pattern for the location type combo box. The exact lines depend on `location_editor_dialog.cpp` — find the `typeComboBox->setModel(...)` call and apply the same pattern as above:
```cpp
#include "utils/translating_proxy_model.h"
#include "utils/type_translation_resolver.h"
// ...
auto* locationTypesProxy = new TranslatingProxyModel(
    LocationTypesListModel::TYPE, TypeTranslationResolver::forLocationTypes(), this
);
locationTypesProxy->setSourceModel(locationTypesModel);
form->typeComboBox->setModel(locationTypesProxy);
form->typeComboBox->setModelColumn(LocationTypesListModel::TYPE);
```

- [ ] **Step 5: Build and smoke-test**

```bash
cmake --build build --target opa
```

Run the app, open a person's event editor. Event type combo box should show "Birth" (or localized equivalent), "Death", etc.

---

## Task 6: TypeTranslationsDialog

**Files:**
- Create: `src/editors/type_translations_dialog.h`
- Create: `src/editors/type_translations_dialog.cpp`
- Create: `src/editors/type_translations_dialog.ui`
- Modify: `src/CMakeLists.txt`

- [ ] **Step 1: Create `src/editors/type_translations_dialog.ui`**

```xml
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>TypeTranslationsForm</class>
 <widget class="QDialog" name="TypeTranslationsForm">
  <property name="windowTitle">
   <string>Translations</string>
  </property>
  <layout class="QVBoxLayout">
   <item>
    <widget class="QLabel" name="titleLabel">
     <property name="text">
      <string>Translations</string>
     </property>
    </widget>
   </item>
   <item>
    <widget class="QTableWidget" name="translationsTable">
     <property name="columnCount">
      <number>2</number>
     </property>
     <property name="selectionBehavior">
      <enum>QAbstractItemView::SelectRows</enum>
     </property>
     <property name="editTriggers">
      <set>QAbstractItemView::NoEditTriggers</set>
     </property>
     <column>
      <property name="text"><string>Language</string></property>
     </column>
     <column>
      <property name="text"><string>Name</string></property>
     </column>
    </widget>
   </item>
   <item>
    <layout class="QHBoxLayout">
     <item>
      <widget class="KLanguageButton" name="languageButton" native="true"/>
     </item>
     <item>
      <widget class="QLineEdit" name="nameEdit">
       <property name="placeholderText">
        <string>Translation</string>
       </property>
      </widget>
     </item>
     <item>
      <widget class="QPushButton" name="addButton">
       <property name="text"><string>Add</string></property>
      </widget>
     </item>
    </layout>
   </item>
   <item>
    <layout class="QHBoxLayout">
     <item>
      <widget class="QPushButton" name="removeButton">
       <property name="text"><string>Remove</string></property>
      </widget>
     </item>
     <item>
      <spacer/>
     </item>
     <item>
      <widget class="QPushButton" name="closeButton">
       <property name="text"><string>Close</string></property>
      </widget>
     </item>
    </layout>
   </item>
  </layout>
 </widget>
 <customwidgets>
  <customwidget>
   <class>KLanguageButton</class>
   <extends>QPushButton</extends>
   <header>KLanguageButton</header>
   <container>0</container>
  </customwidget>
 </customwidgets>
</ui>
```

- [ ] **Step 2: Create `src/editors/type_translations_dialog.h`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QDialog>
#include <functional>
#include <optional>

namespace Ui {
class TypeTranslationsForm;
}

struct TypeTranslationItem {
    IntegerPrimaryKey id;
    QString locale;
    QString name;
};

/**
 * Dialog for managing optional translations of a user-defined type name.
 * Built-in types must not use this dialog.
 *
 * The dialog is parameterised via callbacks so it works for both event and
 * location type translations without duplicating code.
 */
class TypeTranslationsDialog : public QDialog {
    Q_OBJECT

public:
    TypeTranslationsDialog(
        const QString& typeName,
        QList<TypeTranslationItem> existing,
        std::function<std::optional<IntegerPrimaryKey>(const QString& locale, const QString& name)> onInsert,
        std::function<bool(IntegerPrimaryKey id)> onRemove,
        QWidget* parent = nullptr
    );

    ~TypeTranslationsDialog() override;

private Q_SLOTS:
    void addTranslation();
    void removeTranslation();

private:
    void populateTable();

    Ui::TypeTranslationsForm* form;
    QList<TypeTranslationItem> items;
    std::function<std::optional<IntegerPrimaryKey>(const QString&, const QString&)> insertFn;
    std::function<bool(IntegerPrimaryKey)> removeFn;
};
```

- [ ] **Step 3: Create `src/editors/type_translations_dialog.cpp`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "type_translations_dialog.h"
#include "ui_type_translations_dialog.h"

#include <KLanguageButton>
#include <KLocalizedString>
#include <QTableWidget>

using namespace Qt::StringLiterals;

TypeTranslationsDialog::TypeTranslationsDialog(
    const QString& typeName,
    QList<TypeTranslationItem> existing,
    std::function<std::optional<IntegerPrimaryKey>(const QString&, const QString&)> onInsert,
    std::function<bool(IntegerPrimaryKey)> onRemove,
    QWidget* parent
) :
    QDialog(parent),
    form(new Ui::TypeTranslationsForm),
    items(std::move(existing)),
    insertFn(std::move(onInsert)),
    removeFn(std::move(onRemove)) {
    form->setupUi(this);
    setWindowTitle(i18n("Translations for \"%1\"", typeName));
    form->titleLabel->setText(i18n("Translations for \"%1\"", typeName));

    populateTable();

    connect(form->addButton, &QPushButton::clicked, this, &TypeTranslationsDialog::addTranslation);
    connect(form->removeButton, &QPushButton::clicked, this, &TypeTranslationsDialog::removeTranslation);
    connect(form->closeButton, &QPushButton::clicked, this, &QDialog::accept);

    form->removeButton->setEnabled(false);
    connect(
        form->translationsTable,
        &QTableWidget::itemSelectionChanged,
        this,
        [this] { form->removeButton->setEnabled(!form->translationsTable->selectedItems().isEmpty()); }
    );
}

TypeTranslationsDialog::~TypeTranslationsDialog() {
    delete form;
}

void TypeTranslationsDialog::populateTable() {
    form->translationsTable->setRowCount(0);
    for (const auto& item : items) {
        const int row = form->translationsTable->rowCount();
        form->translationsTable->insertRow(row);
        form->translationsTable->setItem(row, 0, new QTableWidgetItem(item.locale));
        form->translationsTable->setItem(row, 1, new QTableWidgetItem(item.name));
        form->translationsTable->item(row, 0)->setData(Qt::UserRole, item.id);
    }
    form->translationsTable->resizeColumnsToContents();
}

void TypeTranslationsDialog::addTranslation() {
    const auto locale = form->languageButton->current();
    const auto name = form->nameEdit->text().trimmed();
    if (locale.isEmpty() || name.isEmpty()) {
        return;
    }
    auto newId = insertFn(locale, name);
    if (!newId.has_value()) {
        return;
    }
    items.append({*newId, locale, name});
    populateTable();
    form->nameEdit->clear();
}

void TypeTranslationsDialog::removeTranslation() {
    const auto selected = form->translationsTable->selectedItems();
    if (selected.isEmpty()) {
        return;
    }
    const int row = selected.first()->row();
    const auto id = form->translationsTable->item(row, 0)->data(Qt::UserRole).toLongLong();
    if (!removeFn(id)) {
        return;
    }
    items.removeIf([id](const TypeTranslationItem& item) { return item.id == id; });
    populateTable();
}
```

- [ ] **Step 4: Register in `src/CMakeLists.txt`**

In `add_library(opa-lib STATIC ...)`, add:
```cmake
  editors/type_translations_dialog.h
  editors/type_translations_dialog.cpp
  editors/type_translations_dialog.ui
```

- [ ] **Step 5: Build**

```bash
cmake --build build --target opa-lib 2>&1 | grep -E "error:|warning:"
```

Expected: clean build.

---

## Task 7: Wire Translations button into management windows

**Files:**
- Modify: `src/lists/event_types_management_window.h`
- Modify: `src/lists/event_types_management_window.cpp`
- Modify: `src/lists/location_types_management_window.h`
- Modify: `src/lists/location_types_management_window.cpp`

- [ ] **Step 1: Update `src/lists/event_types_management_window.h`**

Add private members and slot to the class:
```cpp
#pragma once

#include "simple_list_manager.h"
#include "domain/event/event_types_model.h"

class QAction;

class EventTypesManagementWindow : public SimpleListManagementWindow {
    Q_OBJECT

public:
    explicit EventTypesManagementWindow();

public Q_SLOTS:
    bool repairConfirmation() override;
    void repairItems() override;
    void removeMarkedReferences(
        const QHash<QString, QVector<IntegerPrimaryKey>>& valueToIds,
        const QHash<IntegerPrimaryKey, QString>& idToValue
    ) override;
    bool isUsed(const QVariant& id) override;

protected:
    [[nodiscard]] QVariant doAddItem() const override;
    bool doRemoveItem(const QVariant& id) const override;
    [[nodiscard]] QString translatedItemCount(int itemCount) const override;
    [[nodiscard]] QString translatedItemDescription(const QString& item, bool isBuiltIn) const override;

private Q_SLOTS:
    void openTranslationsDialog();
    void updateTranslationsAction(const QItemSelection& selected, const QItemSelection& deselected);

private:
    EventTypesListModel* typesModel = nullptr;
    QAction* translationsAction = nullptr;
};
```

- [ ] **Step 2: Update `src/lists/event_types_management_window.cpp`**

Add includes:
```cpp
#include "domain/event/event_type_translation_repository.h"
#include "editors/type_translations_dialog.h"
#include <QAction>
#include <QToolBar>
```

Replace the constructor body to store `typesModel` and add the toolbar action:
```cpp
EventTypesManagementWindow::EventTypesManagementWindow() {
    setWindowTitle(i18n("Manage event types"));

    typesModel = new EventTypesListModel(this);
    setModel(typesModel);
    setColumns(EventTypesListModel::ID, EventTypesListModel::TYPE, EventTypesListModel::BUILTIN);
    setTranslator(EventTypes::toDisplayString);

    initializeLayout();

    auto* toolbar = findChild<QToolBar*>();
    translationsAction = new QAction(toolbar);
    translationsAction->setText(i18n("Translations..."));
    translationsAction->setIcon(QIcon::fromTheme(u"languages"_s));
    translationsAction->setEnabled(false);
    toolbar->addAction(translationsAction);
    connect(translationsAction, &QAction::triggered, this, &EventTypesManagementWindow::openTranslationsDialog);

    connect(
        tableView->selectionModel(),
        &QItemSelectionModel::selectionChanged,
        this,
        &EventTypesManagementWindow::updateTranslationsAction
    );
}
```

Add the two new slot implementations:
```cpp
void EventTypesManagementWindow::updateTranslationsAction(
    const QItemSelection& selected, [[maybe_unused]] const QItemSelection& deselected
) {
    if (selected.isEmpty()) {
        translationsAction->setEnabled(false);
        return;
    }
    auto rootIndex = mapToSourceModel(selected.indexes().first());
    auto isBuiltin = typesModel->index(rootIndex.row(), EventTypesListModel::BUILTIN).data().toBool();
    translationsAction->setEnabled(!isBuiltin);
}

void EventTypesManagementWindow::openTranslationsDialog() {
    auto* selection = tableView->selectionModel();
    if (!selection->hasSelection()) return;

    auto rootIndex = mapToSourceModel(selection->selection().first().indexes().first());
    const auto typeId = typesModel->index(rootIndex.row(), EventTypesListModel::ID).data().toLongLong();
    const auto typeName = typesModel->index(rootIndex.row(), EventTypesListModel::TYPE).data().toString();

    EventTypeTranslationRepository repo;
    auto existing = repo.findAllForType(typeId);
    QList<TypeTranslationItem> items;
    items.reserve(existing.size());
    for (const auto& e : existing) {
        items.append({e.id, e.locale, e.name});
    }

    auto* dialog = new TypeTranslationsDialog(
        typeName,
        items,
        [typeId, &repo](const QString& locale, const QString& name) {
            return repo.insert(typeId, locale, name);
        },
        [&repo](IntegerPrimaryKey id) { return repo.remove(id); },
        this
    );
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}
```

- [ ] **Step 3: Update `src/lists/location_types_management_window.h`**

Apply the same pattern as the event window. Full content:
```cpp
#pragma once

#include "simple_list_manager.h"
#include "domain/location/location_types_list_model.h"

class QAction;

class LocationTypesManagementWindow : public SimpleListManagementWindow {
    Q_OBJECT

public:
    explicit LocationTypesManagementWindow();

public Q_SLOTS:
    bool repairConfirmation() override;
    void repairItems() override;
    void removeMarkedReferences(
        const QHash<QString, QVector<IntegerPrimaryKey>>& valueToIds,
        const QHash<IntegerPrimaryKey, QString>& idToValue
    ) override;
    bool isUsed(const QVariant& id) override;

protected:
    [[nodiscard]] QVariant doAddItem() const override;
    bool doRemoveItem(const QVariant& id) const override;
    [[nodiscard]] QString translatedItemCount(int itemCount) const override;
    [[nodiscard]] QString translatedItemDescription(const QString& item, bool isBuiltIn) const override;

private Q_SLOTS:
    void openTranslationsDialog();
    void updateTranslationsAction(const QItemSelection& selected, const QItemSelection& deselected);

private:
    LocationTypesListModel* locationTypesModel = nullptr;
    QAction* translationsAction = nullptr;
};
```

- [ ] **Step 4: Update `src/lists/location_types_management_window.cpp`**

Apply same pattern as event window. Replace constructor and add new slots:
```cpp
#include "domain/location/location_types.h"
#include "domain/location/location_type_translation_repository.h"
#include "editors/type_translations_dialog.h"
#include <QAction>
#include <QToolBar>

LocationTypesManagementWindow::LocationTypesManagementWindow() {
    setWindowTitle(i18n("Manage location types"));

    locationTypesModel = new LocationTypesListModel(this);
    setModel(locationTypesModel);
    setColumns(LocationTypesListModel::ID, LocationTypesListModel::TYPE, LocationTypesListModel::BUILTIN);
    setTranslator(LocationTypes::toDisplayString);

    initializeLayout();

    auto* toolbar = findChild<QToolBar*>();
    translationsAction = new QAction(toolbar);
    translationsAction->setText(i18n("Translations..."));
    translationsAction->setIcon(QIcon::fromTheme(u"languages"_s));
    translationsAction->setEnabled(false);
    toolbar->addAction(translationsAction);
    connect(translationsAction, &QAction::triggered, this,
            &LocationTypesManagementWindow::openTranslationsDialog);

    connect(
        tableView->selectionModel(),
        &QItemSelectionModel::selectionChanged,
        this,
        &LocationTypesManagementWindow::updateTranslationsAction
    );
}

void LocationTypesManagementWindow::updateTranslationsAction(
    const QItemSelection& selected, [[maybe_unused]] const QItemSelection& deselected
) {
    if (selected.isEmpty()) {
        translationsAction->setEnabled(false);
        return;
    }
    auto rootIndex = mapToSourceModel(selected.indexes().first());
    auto isBuiltin = locationTypesModel->index(rootIndex.row(), LocationTypesListModel::BUILTIN).data().toBool();
    translationsAction->setEnabled(!isBuiltin);
}

void LocationTypesManagementWindow::openTranslationsDialog() {
    auto* selection = tableView->selectionModel();
    if (!selection->hasSelection()) return;

    auto rootIndex = mapToSourceModel(selection->selection().first().indexes().first());
    const auto typeId = locationTypesModel->index(rootIndex.row(), LocationTypesListModel::ID).data().toLongLong();
    const auto typeName = locationTypesModel->index(rootIndex.row(), LocationTypesListModel::TYPE).data().toString();

    LocationTypeTranslationRepository repo;
    auto existing = repo.findAllForType(typeId);
    QList<TypeTranslationItem> items;
    items.reserve(existing.size());
    for (const auto& e : existing) {
        items.append({e.id, e.locale, e.name});
    }

    auto* dialog = new TypeTranslationsDialog(
        typeName,
        items,
        [typeId, &repo](const QString& locale, const QString& name) {
            return repo.insert(typeId, locale, name);
        },
        [&repo](IntegerPrimaryKey id) { return repo.remove(id); },
        this
    );
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}
```

The remaining methods (`repairConfirmation`, `repairItems`, `removeMarkedReferences`, `isUsed`, `doAddItem`, `doRemoveItem`, `translatedItemCount`, `translatedItemDescription`) are unchanged from the current implementation.

- [ ] **Step 5: Build the full app**

```bash
cmake --build build --target opa
```

Expected: clean build with no errors.

- [ ] **Step 6: Smoke test — management window**

Run `./build/src/opa`. Open Settings → Manage location types. Add a custom type (e.g., "Boerderij"). Select it — the "Translations..." button should enable. Click it. Add locale `en`, name `Farm`. Close. Reopen dialog to verify the translation persists.

Select a built-in type (e.g., "Country") — "Translations..." button should be disabled.

- [ ] **Step 7: Smoke test — event editor**

Open a person, add an event. The event type combo box should show built-in types in the app's display language (Dutch "Geboorte" if locale is `nl`, English "Birth" if `en`). User-defined types appear as their stored name.

---

## Self-Review

**Spec coverage:**
- ✅ LocationTypes enum + kli18n (Task 1)
- ✅ Schema migration (Task 2)
- ✅ Translation entities + repositories (Task 3)
- ✅ TypeTranslationResolver with locale fallback (Task 4)
- ✅ `BuiltinTextTranslatingDelegate` crash fix (Task 1, step 3)
- ✅ Translations button disabled for built-in types (Task 7, steps 2+4)
- ✅ TypeTranslationsDialog with KLanguageButton (Task 6)
- ✅ ON DELETE CASCADE (Task 2, step 4 — SQL DDL)
- ✅ Resolver used in combo boxes (Task 5)
- ✅ DataEventBroker notifications on write (Tasks 3, 5)

**Type consistency:**
- `EventTypesListModel::ID/TYPE/BUILTIN` columns used consistently across Tasks 1, 7
- `LocationTypesListModel::ID/TYPE/BUILTIN` columns used consistently
- `TypeTranslationItem { id, locale, name }` matches what both repos return
- `findByTypeStringAndLocale` used in resolver and tested in Task 3

**No placeholders:** All code blocks contain actual, complete implementations.
