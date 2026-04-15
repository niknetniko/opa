# Locations Feature Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add hierarchical location support to Opa — a normalized `locations` table with type, parent, note, coordinates, and date range — attached to events via a nullable FK.

**Architecture:** New `src/domain/location/` domain following the existing repository/entity/model pattern. UI follows sources: `TreeProxyModel` for tree views, `ChooseExistingReferenceWindow` for the picker, `SimpleListManagementWindow` for the types list. Events gain an optional `locationId` FK wired into the event editor combobox.

**Tech Stack:** C++20, Qt 6 Widgets, SQLite (via QSqlQuery), KDE Frameworks 6 (KLocalizedString, KCollapsibleGroupBox), CMake, CTest/QTest.

**Spec:** `docs/superpowers/specs/2026-04-13-locations-design.md`

---

## File Map

### New files
| File | Purpose |
|---|---|
| `src/database/migrations/002_add_locations.sql` | Migration: create `location_types`, `locations`, add `events.location_id` |
| `src/domain/location/location_entities.h` | `LocationTypeEntity`, `LocationEntity`, `LocationDisplayEntity` structs |
| `src/domain/location/location_repository.h` | `LocationRepository` interface |
| `src/domain/location/location_repository.cpp` | All SQL for location CRUD + recursive path CTE |
| `src/domain/location/location_types_list_model.h/cpp` | `ObjectTableModel<LocationTypeEntity>` for type comboboxes + management window |
| `src/domain/location/location_list_model.h/cpp` | `ObjectTableModel<LocationEntity>` for tree views |
| `src/domain/location/location_paths_model.h/cpp` | `ObjectTableModel<LocationDisplayEntity>` for event editor combobox |
| `src/lists/location_types_management_window.h/cpp` | `SimpleListManagementWindow` subclass for CRUD on location types |
| `src/lists/location_management_window.h/cpp` | `QMainWindow` with `QTreeView` for location CRUD |
| `src/editors/location_editor_dialog.h/cpp` + `.ui` | Dialog to create/edit a location (name, type, parent, note, coords, dates) |
| `src/link_existing/choose_existing_location_window.h/cpp` | `ChooseExistingReferenceWindow` subclass for parent picker |
| `autotests/location_repository_test.cpp` | QTest suite for `LocationRepository` |

### Modified files
| File | Change |
|---|---|
| `src/database/schema.sql` | Add `location_types` + `locations` tables |
| `src/database/init.sql` | Add builtin location types |
| `src/database/schema.h` | Add `Schema::LocationTypes`, `Schema::Locations` |
| `src/database/database.cpp` | Register migration 2 |
| `src/CMakeLists.txt` | Add all new `.h/.cpp` files to `opa-lib`; add migration to `qt_add_resources` |
| `autotests/CMakeLists.txt` | Register `location_repository_test.cpp` |
| `autotests/database_migration_test.cpp` | Update version check; add migration 2 tests |
| `src/utils/formatted_identifier_delegate.h` | Add `LOCATION` constant |
| `src/domain/event/event_entities.h` | Add `locationId` to `EventEntity` |
| `src/domain/event/event_repository.h` | Update `updateEvent` + `insertFullEvent` signatures |
| `src/domain/event/event_repository.cpp` | Implement updated SQL |
| `autotests/event_repository_test.cpp` | Update call sites for changed signatures |
| `src/editors/event_editor_dialog.h` | Add `locationsModel` member |
| `src/editors/event_editor_dialog.cpp` | Wire up location combobox |
| `src/editors/event_editor_dialog.ui` | Add Location row to event information group |

---

## Task 1: Schema, migration, and build registration

**Files:**
- Create: `src/database/migrations/002_add_locations.sql`
- Modify: `src/database/schema.sql`
- Modify: `src/database/init.sql`
- Modify: `src/database/schema.h`
- Modify: `src/database/database.cpp`
- Modify: `src/CMakeLists.txt`
- Modify: `autotests/database_migration_test.cpp`

- [ ] **Step 1: Write failing migration tests**

Add to `autotests/database_migration_test.cpp` after the existing test cases, before the closing `}` of `TestDatabaseMigrations`:

```cpp
// ==================== Migration 2 ====================

void testFreshDatabaseIsStampedWithVersionTwo() {
    openDatabase(u":memory:"_s, false, false);
    auto db = QSqlDatabase::database();
    QCOMPARE(userVersion(db), 2);
}

// Builds a version-1 database: full v1 schema with surrogate-keyed event_relations.
static QSqlDatabase setupVersion1Database() {
    auto db = openRawDatabase();
    QSqlQuery(db).exec(u"PRAGMA foreign_keys = OFF"_s);
    // Supporting tables (same as v1 final state).
    const QStringList ddl = {
        u"CREATE TABLE name_origins (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, origin TEXT, builtin BOOLEAN NOT NULL DEFAULT FALSE)"_s,
        u"CREATE TABLE people (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, root BOOLEAN, sex TEXT)"_s,
        u"CREATE TABLE names (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, person_id INTEGER NOT NULL, sort INTEGER NOT NULL, titles TEXT, given_names TEXT, prefix TEXT, surname TEXT, note TEXT, origin_id INTEGER NULL DEFAULT NULL)"_s,
        u"CREATE TABLE event_types (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, type TEXT, builtin BOOLEAN NOT NULL DEFAULT FALSE)"_s,
        u"CREATE TABLE event_roles (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, role TEXT, builtin BOOLEAN NOT NULL DEFAULT FALSE)"_s,
        u"CREATE TABLE events (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, type_id INTEGER NOT NULL, date TEXT, name TEXT, note TEXT)"_s,
        u"CREATE TABLE event_relations (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, event_id INTEGER NOT NULL, person_id INTEGER NOT NULL, role_id INTEGER NOT NULL, UNIQUE (event_id, person_id, role_id))"_s,
        u"CREATE TABLE sources (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, title TEXT, type TEXT, author TEXT, publication TEXT, confidence TEXT NOT NULL, note TEXT, parent_id INTEGER)"_s,
        u"CREATE TABLE event_citations (event_id INTEGER NOT NULL, source_id INTEGER NOT NULL, PRIMARY KEY (event_id, source_id))"_s,
        u"CREATE TABLE event_relation_citations (event_relation_id INTEGER NOT NULL, source_id INTEGER NOT NULL, PRIMARY KEY (event_relation_id, source_id))"_s,
        u"CREATE TABLE name_citations (name_id INTEGER NOT NULL, source_id INTEGER NOT NULL, PRIMARY KEY (name_id, source_id))"_s,
        u"CREATE TABLE person_citations (person_id INTEGER NOT NULL, source_id INTEGER NOT NULL, PRIMARY KEY (person_id, source_id))"_s,
    };
    for (const auto& stmt: ddl) {
        QSqlQuery(db).exec(stmt);
    }
    QSqlQuery(db).exec(u"PRAGMA user_version = 1"_s);
    return db;
}

void testMigration2AddsLocationTypesTable() {
    auto db = setupVersion1Database();
    QVERIFY(!columnExists(db, u"location_types"_s, u"id"_s));

    runMigrations(db);

    QVERIFY(columnExists(db, u"location_types"_s, u"id"_s));
    QCOMPARE(userVersion(db), 2);
}

void testMigration2AddsLocationsTable() {
    auto db = setupVersion1Database();

    runMigrations(db);

    QVERIFY(columnExists(db, u"locations"_s, u"id"_s));
    QVERIFY(columnExists(db, u"locations"_s, u"parent_id"_s));
    QVERIFY(columnExists(db, u"locations"_s, u"type_id"_s));
}

void testMigration2AddsLocationIdToEvents() {
    auto db = setupVersion1Database();
    QVERIFY(!columnExists(db, u"events"_s, u"location_id"_s));

    runMigrations(db);

    QVERIFY(columnExists(db, u"events"_s, u"location_id"_s));
}

void testMigration2PreservesExistingEvents() {
    auto db = setupVersion1Database();
    QSqlQuery(db).exec(u"PRAGMA foreign_keys = OFF"_s);
    QSqlQuery(db).exec(u"INSERT INTO event_types VALUES (1, 'Birth', false)"_s);
    QSqlQuery(db).exec(u"INSERT INTO events VALUES (1, 1, '1900', 'Test', 'Note')"_s);

    runMigrations(db);

    QSqlQuery q(db);
    QVERIFY(q.exec(u"SELECT id, name FROM events"_s));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 1);
    QCOMPARE(q.value(1).toString(), u"Test"_s);
}
```

Also update the existing version check test — find and replace:
```cpp
void testFreshDatabaseIsStampedWithLatestVersion() {
    openDatabase(u":memory:"_s, false, false);
    auto db = QSqlDatabase::database();
    QCOMPARE(userVersion(db), 1);
}
```
with:
```cpp
void testFreshDatabaseIsStampedWithLatestVersion() {
    openDatabase(u":memory:"_s, false, false);
    auto db = QSqlDatabase::database();
    QCOMPARE(userVersion(db), 2);
}
```

Also update the noop test similarly (1 → 2):
```cpp
void testRunMigrationsIsNoopOnCurrentDatabase() {
    openDatabase(u":memory:"_s, false, false);
    auto db = QSqlDatabase::database();
    QCOMPARE(userVersion(db), 2);

    runMigrations(db);

    QCOMPARE(userVersion(db), 2);
}
```

- [ ] **Step 2: Run tests to verify they fail**

```bash
cd build && ctest -R database_migration_test -V 2>&1 | tail -30
```

Expected: FAIL on `testFreshDatabaseIsStampedWithLatestVersion` (got 1, expected 2) and the new migration 2 tests (table does not exist yet).

- [ ] **Step 3: Create the migration SQL**

Create `src/database/migrations/002_add_locations.sql`:

```sql
CREATE TABLE location_types (
  id      INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type    TEXT NOT NULL,
  builtin BOOLEAN NOT NULL DEFAULT FALSE
);

CREATE TABLE locations (
  id         INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  name       TEXT NOT NULL,
  type_id    INTEGER NULL REFERENCES location_types (id) ON DELETE SET NULL,
  parent_id  INTEGER NULL REFERENCES locations (id) ON DELETE SET NULL,
  note       TEXT,
  latitude   REAL,
  longitude  REAL,
  date_start TEXT,
  date_end   TEXT
);

ALTER TABLE events ADD COLUMN location_id INTEGER NULL REFERENCES locations (id) ON DELETE SET NULL
```

- [ ] **Step 4: Update `schema.sql` to reflect the final schema**

Add after the `events` table definition in `src/database/schema.sql`:

```sql
CREATE TABLE location_types (
  id      INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type    TEXT NOT NULL,
  builtin BOOLEAN NOT NULL DEFAULT FALSE
);

CREATE TABLE locations (
  id         INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  name       TEXT NOT NULL,
  type_id    INTEGER NULL REFERENCES location_types (id) ON DELETE SET NULL,
  parent_id  INTEGER NULL REFERENCES locations (id) ON DELETE SET NULL,
  note       TEXT,
  latitude   REAL,
  longitude  REAL,
  date_start TEXT,
  date_end   TEXT
);
```

Also change the `events` table to include `location_id`:

```sql
CREATE TABLE events (
  id          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type_id     INTEGER NOT NULL REFERENCES event_types (id) ON DELETE RESTRICT,
  date        TEXT,
  name        TEXT,
  note        TEXT,
  location_id INTEGER NULL REFERENCES locations (id) ON DELETE SET NULL
);
```

- [ ] **Step 5: Add builtin location types to `init.sql`**

Append to `src/database/init.sql`:

```sql
INSERT INTO
  location_types (type, builtin)
VALUES
  ('Country', true),
  ('Province', true),
  ('County', true),
  ('City', true),
  ('Village', true),
  ('Parish', true),
  ('Address', true);
```

- [ ] **Step 6: Add schema tags to `schema.h`**

In `src/database/schema.h`, add before the closing `}` of the `Schema` namespace:

```cpp
struct LocationTypes : TableTag {
    static constexpr auto table = QLatin1String("location_types");
};
inline constexpr auto LocationTypesTable = LocationTypes::table;

struct Locations : TableTag {
    static constexpr auto table = QLatin1String("locations");
};
inline constexpr auto LocationsTable = Locations::table;
```

- [ ] **Step 7: Register migration 2 in `database.cpp`**

In `src/database/database.cpp`, change:

```cpp
constexpr std::array migrations = {Migration{
    .version = 1,
    .description = "Add surrogate key to event_relations, rekey event_relation_citations"_L1,
    .resourcePath = ":/migrations/001_event_relations_surrogate_key.sql"_L1,
}};
```

to:

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
};
```

- [ ] **Step 8: Register migration in `CMakeLists.txt` resources**

In `src/CMakeLists.txt`, change:

```cmake
qt_add_resources(
  opa-lib "opa-database"
  PREFIX "/"
  BASE "database"
  FILES database/init.sql database/schema.sql database/seed.sql
        database/migrations/001_event_relations_surrogate_key.sql)
```

to:

```cmake
qt_add_resources(
  opa-lib "opa-database"
  PREFIX "/"
  BASE "database"
  FILES database/init.sql database/schema.sql database/seed.sql
        database/migrations/001_event_relations_surrogate_key.sql
        database/migrations/002_add_locations.sql)
```

- [ ] **Step 9: Build and run tests**

```bash
cd build && cmake .. && make -j$(nproc) && ctest -R database_migration_test -V 2>&1 | tail -30
```

Expected: all `database_migration_test` cases PASS.

- [ ] **Step 10: Commit**

```bash
git add src/database/migrations/002_add_locations.sql \
        src/database/schema.sql src/database/init.sql \
        src/database/schema.h src/database/database.cpp \
        src/CMakeLists.txt autotests/database_migration_test.cpp
git commit -m "feat: add location_types and locations schema + migration 002"
```

---

## Task 2: Location entities and repository

**Files:**
- Create: `src/domain/location/location_entities.h`
- Create: `src/domain/location/location_repository.h`
- Create: `src/domain/location/location_repository.cpp`
- Create: `autotests/location_repository_test.cpp`
- Modify: `src/CMakeLists.txt`
- Modify: `autotests/CMakeLists.txt`

- [ ] **Step 1: Write the failing tests**

Create `autotests/location_repository_test.cpp`:

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
#include "../src/domain/location/location_repository.h"

#include "./test_utils.h"
#include "database/database.h"
#include "database/schema.h"

#include <QSqlDatabase>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestLocationRepository : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void init() {
        QVERIFY(QSqlDatabase::isDriverAvailable(u"QSQLITE"_s));
        openDatabase(u":memory:"_s, false);
    }

    void cleanup() {
        QSqlDatabase::database().close();
    }

    // LocationType tests

    void testInsertAndFindLocationTypeById() {
        LocationRepository repo;
        auto id = repo.insertLocationType(u"City"_s);
        QVERIFY(id.has_value());

        auto result = repo.findLocationTypeById(*id);
        QVERIFY(result.has_value());
        QCOMPARE(result->id, *id);
        QCOMPARE(result->type, u"City"_s);
        QCOMPARE(result->builtin, false);
    }

    void testFindAllLocationTypes() {
        LocationRepository repo;
        const auto before = repo.findAllLocationTypes().size();
        repo.insertLocationType(u"TypeA"_s);
        repo.insertLocationType(u"TypeB"_s);

        auto types = repo.findAllLocationTypes();
        QCOMPARE(types.size(), before + 2);
    }

    void testUpdateLocationType() {
        LocationRepository repo;
        auto id = repo.insertLocationType(u"City"_s);
        QVERIFY(id.has_value());

        QVERIFY(repo.updateLocationType(*id, u"Town"_s));

        auto result = repo.findLocationTypeById(*id);
        QVERIFY(result.has_value());
        QCOMPARE(result->type, u"Town"_s);
    }

    void testDeleteLocationType() {
        LocationRepository repo;
        auto id = repo.insertLocationType(u"City"_s);
        QVERIFY(id.has_value());

        QVERIFY(repo.deleteLocationType(*id));
        QVERIFY(!repo.findLocationTypeById(*id).has_value());
    }

    void testIsLocationTypeUsed() {
        LocationRepository repo;
        auto typeId = repo.insertLocationType(u"City"_s);
        QVERIFY(typeId.has_value());

        QVERIFY(!repo.isLocationTypeUsed(*typeId));

        auto locId = repo.insert(u"Amsterdam"_s, *typeId, std::nullopt);
        QVERIFY(locId.has_value());

        QVERIFY(repo.isLocationTypeUsed(*typeId));
    }

    // Location tests

    void testInsertAndFindById() {
        LocationRepository repo;
        auto id = repo.insert(u"Netherlands"_s, std::nullopt, std::nullopt);
        QVERIFY(id.has_value());

        auto result = repo.findById(*id);
        QVERIFY(result.has_value());
        QCOMPARE(result->id, *id);
        QCOMPARE(result->name, u"Netherlands"_s);
        QVERIFY(!result->typeId.has_value());
        QVERIFY(!result->parentId.has_value());
    }

    void testInsertWithParent() {
        LocationRepository repo;
        auto parentId = repo.insert(u"Netherlands"_s, std::nullopt, std::nullopt);
        QVERIFY(parentId.has_value());

        auto childId = repo.insert(u"Groningen"_s, std::nullopt, *parentId);
        QVERIFY(childId.has_value());

        auto result = repo.findById(*childId);
        QVERIFY(result.has_value());
        QVERIFY(result->parentId.has_value());
        QCOMPARE(*result->parentId, *parentId);
    }

    void testUpdate() {
        LocationRepository repo;
        auto id = repo.insert(u"Netherlands"_s, std::nullopt, std::nullopt);
        QVERIFY(id.has_value());

        QVERIFY(repo.update(*id, u"The Netherlands"_s, std::nullopt, std::nullopt,
                            u"A note"_s, 52.3, 4.9, u"1815"_s, QString{}));

        auto result = repo.findById(*id);
        QVERIFY(result.has_value());
        QCOMPARE(result->name, u"The Netherlands"_s);
        QCOMPARE(result->note, u"A note"_s);
        QVERIFY(result->latitude.has_value());
        QCOMPARE(*result->latitude, 52.3);
        QCOMPARE(result->dateStart, u"1815"_s);
        QVERIFY(result->dateEnd.isEmpty());
    }

    void testDeleteLocation() {
        LocationRepository repo;
        auto id = repo.insert(u"Netherlands"_s, std::nullopt, std::nullopt);
        QVERIFY(id.has_value());

        QVERIFY(repo.deleteLocation(*id));
        QVERIFY(!repo.findById(*id).has_value());
    }

    void testIsUsed() {
        LocationRepository repo;
        auto locId = repo.insert(u"Amsterdam"_s, std::nullopt, std::nullopt);
        QVERIFY(locId.has_value());
        QVERIFY(!repo.isUsed(*locId));

        // Attach to an event (insert event_type + event manually).
        auto typeId = insertQuery(u"INSERT INTO event_types (type, builtin) VALUES ('Birth', false)"_s);
        auto eventId = insertQuery(
            u"INSERT INTO events (type_id, location_id) VALUES (%1, %2)"_s.arg(typeId).arg(*locId)
        );
        Q_UNUSED(eventId);

        QVERIFY(repo.isUsed(*locId));
    }

    void testFindAllWithPaths_singleRoot() {
        LocationRepository repo;
        auto id = repo.insert(u"Netherlands"_s, std::nullopt, std::nullopt);
        QVERIFY(id.has_value());

        auto paths = repo.findAllWithPaths();
        auto it = std::find_if(paths.begin(), paths.end(), [&](const auto& e) { return e.id == *id; });
        QVERIFY(it != paths.end());
        QCOMPARE(it->fullPath, u"Netherlands"_s);
    }

    void testFindAllWithPaths_childPath() {
        LocationRepository repo;
        auto nlId = repo.insert(u"Netherlands"_s, std::nullopt, std::nullopt);
        QVERIFY(nlId.has_value());
        auto grId = repo.insert(u"Groningen"_s, std::nullopt, *nlId);
        QVERIFY(grId.has_value());

        auto paths = repo.findAllWithPaths();
        auto it = std::find_if(paths.begin(), paths.end(), [&](const auto& e) { return e.id == *grId; });
        QVERIFY(it != paths.end());
        QCOMPARE(it->fullPath, u"Netherlands > Groningen"_s);
    }

    void testFindOrCreate_createsWhenMissing() {
        LocationRepository repo;
        auto id = repo.findOrCreate(u"Amsterdam"_s, std::nullopt, std::nullopt);
        QVERIFY(id.has_value());
        QVERIFY(repo.findById(*id).has_value());
    }

    void testFindOrCreate_findsExisting() {
        LocationRepository repo;
        auto id1 = repo.findOrCreate(u"Amsterdam"_s, std::nullopt, std::nullopt);
        auto id2 = repo.findOrCreate(u"Amsterdam"_s, std::nullopt, std::nullopt);
        QVERIFY(id1.has_value());
        QVERIFY(id2.has_value());
        QCOMPARE(*id1, *id2);
    }
};

QTEST_MAIN(TestLocationRepository)

#include "location_repository_test.moc"
```

- [ ] **Step 2: Register the test in `autotests/CMakeLists.txt`**

Add `location_repository_test.cpp` to the `ecm_add_tests` call:

```cmake
ecm_add_tests(
  ...existing files...
  location_repository_test.cpp
  LINK_LIBRARIES opa-lib Qt::Test)
```

- [ ] **Step 3: Run tests to verify they fail (won't compile)**

```bash
cd build && cmake .. && make -j$(nproc) 2>&1 | grep "error:" | head -20
```

Expected: compile errors — `location_repository.h` does not exist yet.

- [ ] **Step 4: Create `location_entities.h`**

Create `src/domain/location/location_entities.h`:

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "core/query_utils.h"
#include "database/schema.h"

#include <QSqlQuery>
#include <QString>
#include <optional>

using namespace Qt::StringLiterals;

struct LocationTypeEntity {
    IntegerPrimaryKey id = -1;
    QString type;
    bool builtin = false;

    static LocationTypeEntity fromSql(const QSqlQuery& query) {
        return {
            .id = query.value(u"id").toLongLong(),
            .type = query.value(u"type").toString(),
            .builtin = query.value(u"builtin").toBool(),
        };
    }
};

struct LocationEntity {
    IntegerPrimaryKey id = -1;
    QString name;
    std::optional<IntegerPrimaryKey> typeId;
    std::optional<IntegerPrimaryKey> parentId;
    QString note;
    std::optional<double> latitude;
    std::optional<double> longitude;
    QString dateStart;
    QString dateEnd;

    static LocationEntity fromSql(const QSqlQuery& query) {
        return {
            .id = query.value(u"id").toLongLong(),
            .name = query.value(u"name").toString(),
            .typeId = validOrOptional<IntegerPrimaryKey>(query.value(u"type_id")),
            .parentId = validOrOptional<IntegerPrimaryKey>(query.value(u"parent_id")),
            .note = query.value(u"note").toString(),
            .latitude = validOrOptional<double>(query.value(u"latitude")),
            .longitude = validOrOptional<double>(query.value(u"longitude")),
            .dateStart = query.value(u"date_start").toString(),
            .dateEnd = query.value(u"date_end").toString(),
        };
    }
};

// Used in comboboxes — full ancestral path resolved via recursive CTE.
struct LocationDisplayEntity {
    IntegerPrimaryKey id = -1;
    QString name;
    QString fullPath; // e.g. "Netherlands > Groningen"

    static LocationDisplayEntity fromSql(const QSqlQuery& query) {
        return {
            .id = query.value(u"id").toLongLong(),
            .name = query.value(u"name").toString(),
            .fullPath = query.value(u"full_path").toString(),
        };
    }
};
```

- [ ] **Step 5: Create `location_repository.h`**

Create `src/domain/location/location_repository.h`:

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "core/base_repository.h"
#include "database/schema.h"
#include "location_entities.h"

#include <QList>
#include <optional>

class LocationRepository : public BaseRepository {
public:
    // Location type methods
    [[nodiscard]] QList<LocationTypeEntity> findAllLocationTypes() const;
    [[nodiscard]] std::optional<LocationTypeEntity> findLocationTypeById(IntegerPrimaryKey id) const;
    std::optional<IntegerPrimaryKey> insertLocationType(const QString& type) const;
    bool updateLocationType(IntegerPrimaryKey id, const QString& type) const;
    bool deleteLocationType(IntegerPrimaryKey id) const;
    [[nodiscard]] bool isLocationTypeUsed(IntegerPrimaryKey id) const;

    // Location methods
    [[nodiscard]] QList<LocationEntity> findAll() const;
    [[nodiscard]] QList<LocationDisplayEntity> findAllWithPaths() const;
    [[nodiscard]] std::optional<LocationEntity> findById(IntegerPrimaryKey id) const;
    std::optional<IntegerPrimaryKey>
    insert(const QString& name, std::optional<IntegerPrimaryKey> typeId, std::optional<IntegerPrimaryKey> parentId) const;
    bool update(
        IntegerPrimaryKey id,
        const QString& name,
        std::optional<IntegerPrimaryKey> typeId,
        std::optional<IntegerPrimaryKey> parentId,
        const QString& note,
        std::optional<double> latitude,
        std::optional<double> longitude,
        const QString& dateStart,
        const QString& dateEnd
    ) const;
    bool deleteLocation(IntegerPrimaryKey id) const;
    [[nodiscard]] bool isUsed(IntegerPrimaryKey id) const;
    std::optional<IntegerPrimaryKey>
    findOrCreate(const QString& name, std::optional<IntegerPrimaryKey> typeId, std::optional<IntegerPrimaryKey> parentId) const;
};
```

- [ ] **Step 6: Create `location_repository.cpp`**

Create `src/domain/location/location_repository.cpp`:

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "location_repository.h"

#include "core/data_event_broker.h"
#include "core/query_helper.h"

using namespace Qt::StringLiterals;

// ── Location types ────────────────────────────────────────────────────────────

QList<LocationTypeEntity> LocationRepository::findAllLocationTypes() const {
    return fetchAll<LocationTypeEntity>(u"SELECT id, type, builtin FROM location_types ORDER BY type"_s);
}

std::optional<LocationTypeEntity> LocationRepository::findLocationTypeById(IntegerPrimaryKey id) const {
    return fetchOne<LocationTypeEntity>(
        u"SELECT id, type, builtin FROM location_types WHERE id = :id"_s, {{u":id"_s, id}}
    );
}

std::optional<IntegerPrimaryKey> LocationRepository::insertLocationType(const QString& type) const {
    auto newId = QueryHelper::insert(
        u"INSERT INTO location_types (type, builtin) VALUES (:type, false)"_s, {{u":type"_s, type}}
    );
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::LocationTypes>(*newId);
    }
    return newId;
}

bool LocationRepository::updateLocationType(IntegerPrimaryKey id, const QString& type) const {
    return QueryHelper::executeAndNotify<Schema::LocationTypes>(
        id,
        u"UPDATE location_types SET type = :type WHERE id = :id"_s,
        {{u":type"_s, type}, {u":id"_s, id}}
    );
}

bool LocationRepository::deleteLocationType(IntegerPrimaryKey id) const {
    return QueryHelper::executeAndNotify<Schema::LocationTypes>(
        id,
        u"DELETE FROM location_types WHERE id = :id"_s,
        {{u":id"_s, id}}
    );
}

bool LocationRepository::isLocationTypeUsed(IntegerPrimaryKey id) const {
    auto result = fetchOne<LocationTypeEntity>(
        u"SELECT lt.id, lt.type, lt.builtin FROM location_types lt "
        u"INNER JOIN locations l ON l.type_id = lt.id WHERE lt.id = :id LIMIT 1"_s,
        {{u":id"_s, id}}
    );
    return result.has_value();
}

// ── Locations ─────────────────────────────────────────────────────────────────

QList<LocationEntity> LocationRepository::findAll() const {
    return fetchAll<LocationEntity>(
        u"SELECT id, name, type_id, parent_id, note, latitude, longitude, date_start, date_end "
        u"FROM locations ORDER BY name"_s
    );
}

QList<LocationDisplayEntity> LocationRepository::findAllWithPaths() const {
    const auto sql =
        u"WITH RECURSIVE path(id, name, type_id, parent_id, full_path) AS ("
        u"  SELECT id, name, type_id, parent_id, name FROM locations WHERE parent_id IS NULL"
        u"  UNION ALL"
        u"  SELECT l.id, l.name, l.type_id, l.parent_id, path.full_path || ' > ' || l.name"
        u"  FROM locations l JOIN path ON l.parent_id = path.id"
        u") SELECT id, name, full_path FROM path ORDER BY full_path"_s;
    return fetchAll<LocationDisplayEntity>(sql);
}

std::optional<LocationEntity> LocationRepository::findById(IntegerPrimaryKey id) const {
    return fetchOne<LocationEntity>(
        u"SELECT id, name, type_id, parent_id, note, latitude, longitude, date_start, date_end "
        u"FROM locations WHERE id = :id"_s,
        {{u":id"_s, id}}
    );
}

std::optional<IntegerPrimaryKey> LocationRepository::insert(
    const QString& name,
    std::optional<IntegerPrimaryKey> typeId,
    std::optional<IntegerPrimaryKey> parentId
) const {
    auto newId = QueryHelper::insert(
        u"INSERT INTO locations (name, type_id, parent_id) VALUES (:name, :type_id, :parent_id)"_s,
        {
            {u":name"_s, name},
            {u":type_id"_s, typeId.has_value() ? QVariant(*typeId) : QVariant{}},
            {u":parent_id"_s, parentId.has_value() ? QVariant(*parentId) : QVariant{}},
        }
    );
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::Locations>(*newId);
    }
    return newId;
}

bool LocationRepository::update(
    IntegerPrimaryKey id,
    const QString& name,
    std::optional<IntegerPrimaryKey> typeId,
    std::optional<IntegerPrimaryKey> parentId,
    const QString& note,
    std::optional<double> latitude,
    std::optional<double> longitude,
    const QString& dateStart,
    const QString& dateEnd
) const {
    return QueryHelper::executeAndNotify<Schema::Locations>(
        id,
        u"UPDATE locations SET name = :name, type_id = :type_id, parent_id = :parent_id, "
        u"note = :note, latitude = :latitude, longitude = :longitude, "
        u"date_start = :date_start, date_end = :date_end WHERE id = :id"_s,
        {
            {u":name"_s, name},
            {u":type_id"_s, typeId.has_value() ? QVariant(*typeId) : QVariant{}},
            {u":parent_id"_s, parentId.has_value() ? QVariant(*parentId) : QVariant{}},
            {u":note"_s, note},
            {u":latitude"_s, latitude.has_value() ? QVariant(*latitude) : QVariant{}},
            {u":longitude"_s, longitude.has_value() ? QVariant(*longitude) : QVariant{}},
            {u":date_start"_s, dateStart},
            {u":date_end"_s, dateEnd},
            {u":id"_s, id},
        }
    );
}

bool LocationRepository::deleteLocation(IntegerPrimaryKey id) const {
    return QueryHelper::executeAndNotify<Schema::Locations>(
        id,
        u"DELETE FROM locations WHERE id = :id"_s,
        {{u":id"_s, id}}
    );
}

bool LocationRepository::isUsed(IntegerPrimaryKey id) const {
    auto result = fetchOne<LocationEntity>(
        u"SELECT l.id, l.name, l.type_id, l.parent_id, l.note, l.latitude, l.longitude, "
        u"l.date_start, l.date_end FROM locations l "
        u"INNER JOIN events e ON e.location_id = l.id WHERE l.id = :id LIMIT 1"_s,
        {{u":id"_s, id}}
    );
    return result.has_value();
}

std::optional<IntegerPrimaryKey> LocationRepository::findOrCreate(
    const QString& name,
    std::optional<IntegerPrimaryKey> typeId,
    std::optional<IntegerPrimaryKey> parentId
) const {
    // Try exact match first.
    const auto existing = fetchOne<LocationEntity>(
        u"SELECT id, name, type_id, parent_id, note, latitude, longitude, date_start, date_end "
        u"FROM locations WHERE name = :name "
        u"AND (parent_id IS :parent_id OR (parent_id IS NULL AND :parent_id IS NULL))"_s,
        {
            {u":name"_s, name},
            {u":parent_id"_s, parentId.has_value() ? QVariant(*parentId) : QVariant{}},
        }
    );
    if (existing) {
        return existing->id;
    }
    return insert(name, typeId, parentId);
}
```

- [ ] **Step 7: Register new files in `src/CMakeLists.txt`**

Add to the `opa-lib` source list in `src/CMakeLists.txt`:

```cmake
  domain/location/location_entities.h
  domain/location/location_repository.h
  domain/location/location_repository.cpp
```

- [ ] **Step 8: Build and run tests**

```bash
cd build && cmake .. && make -j$(nproc) && ctest -R location_repository_test -V 2>&1 | tail -40
```

Expected: all `location_repository_test` cases PASS.

- [ ] **Step 9: Commit**

```bash
git add src/domain/location/location_entities.h \
        src/domain/location/location_repository.h \
        src/domain/location/location_repository.cpp \
        autotests/location_repository_test.cpp \
        autotests/CMakeLists.txt \
        src/CMakeLists.txt
git commit -m "feat: add LocationRepository with type and location CRUD"
```

---

## Task 3: Location models

**Files:**
- Create: `src/domain/location/location_types_list_model.h/cpp`
- Create: `src/domain/location/location_list_model.h/cpp`
- Create: `src/domain/location/location_paths_model.h/cpp`
- Modify: `src/CMakeLists.txt`

- [ ] **Step 1: Create `location_types_list_model.h`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "location_entities.h"
#include "model/object_table_model.h"

class LocationTypesListModel : public ObjectTableModel<LocationTypeEntity> {
    Q_OBJECT
public:
    enum Columns { ID = 0, TYPE, BUILTIN };
    Q_ENUM(Columns)

    explicit LocationTypesListModel(QObject* parent = nullptr);

public Q_SLOTS:
    void reload();
};
```

- [ ] **Step 2: Create `location_types_list_model.cpp`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "location_types_list_model.h"

#include "core/data_event_broker.h"
#include "location_repository.h"

#include <KLocalizedString>

LocationTypesListModel::LocationTypesListModel(QObject* parent) : ObjectTableModel(parent) {
    this->setColumn(ID, i18n("ID"), &LocationTypeEntity::id);
    this->setColumn(TYPE, i18n("Type"), &LocationTypeEntity::type);
    this->setColumn(BUILTIN, i18n("Built-in"), &LocationTypeEntity::builtin);
    connectToTable<Schema::LocationTypes>(this);
    reload();
}

void LocationTypesListModel::reload() {
    LocationRepository repo;
    this->setItems(repo.findAllLocationTypes());
}
```

- [ ] **Step 3: Create `location_list_model.h`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "location_entities.h"
#include "model/object_table_model.h"

class LocationListModel : public ObjectTableModel<LocationEntity> {
    Q_OBJECT
public:
    enum Columns { ID = 0, NAME, TYPE_ID, PARENT_ID };
    Q_ENUM(Columns)

    explicit LocationListModel(QObject* parent = nullptr);

public Q_SLOTS:
    void reload();
};
```

- [ ] **Step 4: Create `location_list_model.cpp`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "location_list_model.h"

#include "core/data_event_broker.h"
#include "location_repository.h"

#include <KLocalizedString>

LocationListModel::LocationListModel(QObject* parent) : ObjectTableModel(parent) {
    this->setColumn(ID, i18n("ID"), &LocationEntity::id);
    this->setColumn(NAME, i18n("Name"), &LocationEntity::name);
    this->setColumn(TYPE_ID, i18n("Type"), [](const LocationEntity& e) -> QVariant {
        return e.typeId.has_value() ? QVariant(*e.typeId) : QVariant{};
    });
    this->setColumn(PARENT_ID, i18n("Parent"), [](const LocationEntity& e) -> QVariant {
        return e.parentId.has_value() ? QVariant(*e.parentId) : QVariant{};
    });
    connectToTable<Schema::Locations>(this);
    reload();
}

void LocationListModel::reload() {
    LocationRepository repo;
    this->setItems(repo.findAll());
}
```

- [ ] **Step 5: Create `location_paths_model.h`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "location_entities.h"
#include "model/object_table_model.h"

class LocationPathsModel : public ObjectTableModel<LocationDisplayEntity> {
    Q_OBJECT
public:
    enum Columns { ID = 0, FULL_PATH };
    Q_ENUM(Columns)

    explicit LocationPathsModel(QObject* parent = nullptr);

public Q_SLOTS:
    void reload();
};
```

- [ ] **Step 6: Create `location_paths_model.cpp`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "location_paths_model.h"

#include "core/data_event_broker.h"
#include "location_repository.h"

#include <KLocalizedString>

LocationPathsModel::LocationPathsModel(QObject* parent) : ObjectTableModel(parent) {
    this->setColumn(ID, i18n("ID"), &LocationDisplayEntity::id);
    this->setColumn(FULL_PATH, i18n("Location"), &LocationDisplayEntity::fullPath);
    connectToTable<Schema::Locations>(this);
    reload();
}

void LocationPathsModel::reload() {
    LocationRepository repo;
    auto items = repo.findAllWithPaths();
    // Prepend empty sentinel (id = -1) so the combobox can represent "no location".
    items.prepend(LocationDisplayEntity{.id = -1, .name = {}, .fullPath = {}});
    this->setItems(items);
}
```

- [ ] **Step 7: Register models in `src/CMakeLists.txt`**

Add to the `opa-lib` source list:

```cmake
  domain/location/location_types_list_model.h
  domain/location/location_types_list_model.cpp
  domain/location/location_list_model.h
  domain/location/location_list_model.cpp
  domain/location/location_paths_model.h
  domain/location/location_paths_model.cpp
```

- [ ] **Step 8: Build**

```bash
cd build && cmake .. && make -j$(nproc) 2>&1 | grep -E "error:|warning:" | head -20
```

Expected: clean build, no errors.

- [ ] **Step 9: Commit**

```bash
git add src/domain/location/location_types_list_model.h \
        src/domain/location/location_types_list_model.cpp \
        src/domain/location/location_list_model.h \
        src/domain/location/location_list_model.cpp \
        src/domain/location/location_paths_model.h \
        src/domain/location/location_paths_model.cpp \
        src/CMakeLists.txt
git commit -m "feat: add location list models (types, tree, paths)"
```

---

## Task 4: LocationTypesManagementWindow

**Files:**
- Create: `src/lists/location_types_management_window.h/cpp`
- Modify: `src/CMakeLists.txt`

- [ ] **Step 1: Create `location_types_management_window.h`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "simple_list_manager.h"

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
};
```

- [ ] **Step 2: Create `location_types_management_window.cpp`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "location_types_management_window.h"

#include "domain/location/location_repository.h"
#include "domain/location/location_types_list_model.h"

#include <KLocalizedString>
#include <QMessageBox>

LocationTypesManagementWindow::LocationTypesManagementWindow() {
    setWindowTitle(i18n("Manage location types"));

    auto* model = new LocationTypesListModel(this);
    setModel(model);
    setColumns(LocationTypesListModel::ID, LocationTypesListModel::TYPE, LocationTypesListModel::BUILTIN);

    initializeLayout();
}

bool LocationTypesManagementWindow::repairConfirmation() {
    return QMessageBox::question(
               this,
               i18n("Clean up location types?"),
               i18n("This will merge duplicate and remove empty location types. Continue?")
           ) == QMessageBox::Yes;
}

void LocationTypesManagementWindow::repairItems() {
    if (!repairConfirmation()) {
        return;
    }
    LocationRepository repo;
    const auto allTypes = repo.findAllLocationTypes();
    for (const auto& entity: allTypes) {
        auto trimmed = entity.type.simplified();
        if (!trimmed.isEmpty()) {
            trimmed[0] = trimmed[0].toTitleCase();
        }
        if (trimmed != entity.type) {
            repo.updateLocationType(entity.id, trimmed);
        }
    }
}

void LocationTypesManagementWindow::removeMarkedReferences(
    [[maybe_unused]] const QHash<QString, QVector<IntegerPrimaryKey>>& valueToIds,
    [[maybe_unused]] const QHash<IntegerPrimaryKey, QString>& idToValue
) {
    // repairItems() is overridden to use the repository directly.
}

bool LocationTypesManagementWindow::isUsed(const QVariant& id) {
    LocationRepository repo;
    return repo.isLocationTypeUsed(id.toLongLong());
}

QVariant LocationTypesManagementWindow::doAddItem() const {
    LocationRepository repo;
    const auto newId = repo.insertLocationType(QString{});
    return newId ? QVariant(*newId) : QVariant{};
}

bool LocationTypesManagementWindow::doRemoveItem(const QVariant& id) const {
    LocationRepository repo;
    return repo.deleteLocationType(id.toLongLong());
}

QString LocationTypesManagementWindow::translatedItemCount(int itemCount) const {
    return i18np("%1 location type", "%1 location types", itemCount);
}

QString LocationTypesManagementWindow::translatedItemDescription(const QString& item, bool isBuiltIn) const {
    if (isBuiltIn) {
        return i18n("Built-in location type '%1'", item);
    }
    return i18n("Location type '%1'", item);
}
```

- [ ] **Step 3: Register in `src/CMakeLists.txt`**

```cmake
  lists/location_types_management_window.h
  lists/location_types_management_window.cpp
```

- [ ] **Step 4: Build**

```bash
cd build && cmake .. && make -j$(nproc) 2>&1 | grep "error:" | head -20
```

Expected: clean build.

- [ ] **Step 5: Commit**

```bash
git add src/lists/location_types_management_window.h \
        src/lists/location_types_management_window.cpp \
        src/CMakeLists.txt
git commit -m "feat: add LocationTypesManagementWindow"
```

---

## Task 5: ChooseExistingLocationWindow and LOCATION identifier

**Files:**
- Modify: `src/utils/formatted_identifier_delegate.h`
- Create: `src/link_existing/choose_existing_location_window.h/cpp`
- Modify: `src/CMakeLists.txt`

- [ ] **Step 1: Add LOCATION identifier constant**

In `src/utils/formatted_identifier_delegate.h`, add after the existing constants:

```cpp
static constexpr auto LOCATION = QLatin1String("L%1");
```

- [ ] **Step 2: Create `choose_existing_location_window.h`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "choose_existing_reference_window.h"

class ChooseExistingLocationWindow : public ChooseExistingReferenceWindow {
    Q_OBJECT

public:
    static QVariant selectLocation(QWidget* parent);

protected:
    explicit ChooseExistingLocationWindow(QWidget* parent);
};
```

- [ ] **Step 3: Create `choose_existing_location_window.cpp`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "choose_existing_location_window.h"

#include "domain/location/location_paths_model.h"
#include "utils/formatted_identifier_delegate.h"

#include <KLocalizedString>
#include <KRearrangeColumnsProxyModel>
#include <QHeaderView>
#include <QLabel>

QVariant ChooseExistingLocationWindow::selectLocation(QWidget* parent) {
    ChooseExistingLocationWindow dialog(parent);
    dialog.exec();
    return dialog.selected;
}

ChooseExistingLocationWindow::ChooseExistingLocationWindow(QWidget* parent) :
    ChooseExistingReferenceWindow(
        LocationPathsModel::FULL_PATH,
        LocationPathsModel::ID,
        new LocationPathsModel(parent),
        parent
    ) {
    setWindowTitle(i18n("Select location"));
    tableHelpText->setText(i18n("Choose an existing location"));
    displayModel->setSourceColumns({LocationPathsModel::ID, LocationPathsModel::FULL_PATH});
    tableView->setItemDelegateForColumn(
        LocationPathsModel::ID, new FormattedIdentifierDelegate(tableView, FormattedIdentifierDelegate::LOCATION)
    );
    tableView->horizontalHeader()->setSectionResizeMode(LocationPathsModel::ID, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}
```

- [ ] **Step 4: Register in `src/CMakeLists.txt`**

```cmake
  link_existing/choose_existing_location_window.h
  link_existing/choose_existing_location_window.cpp
```

- [ ] **Step 5: Build**

```bash
cd build && cmake .. && make -j$(nproc) 2>&1 | grep "error:" | head -20
```

Expected: clean build.

- [ ] **Step 6: Commit**

```bash
git add src/utils/formatted_identifier_delegate.h \
        src/link_existing/choose_existing_location_window.h \
        src/link_existing/choose_existing_location_window.cpp \
        src/CMakeLists.txt
git commit -m "feat: add ChooseExistingLocationWindow and LOCATION identifier"
```

---

## Task 6: LocationEditorDialog

**Files:**
- Create: `src/editors/location_editor_dialog.ui`
- Create: `src/editors/location_editor_dialog.h`
- Create: `src/editors/location_editor_dialog.cpp`
- Modify: `src/CMakeLists.txt`

- [ ] **Step 1: Create `location_editor_dialog.ui`**

```xml
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>LocationEditorForm</class>
 <widget class="QDialog" name="LocationEditorForm">
  <property name="geometry">
   <rect><x>0</x><y>0</y><width>500</width><height>460</height></rect>
  </property>
  <property name="windowTitle"><string>Form</string></property>
  <layout class="QVBoxLayout" name="verticalLayout">
   <item>
    <widget class="QGroupBox" name="groupBox">
     <property name="title"><string>Location information</string></property>
     <layout class="QFormLayout" name="formLayout">
      <item row="0" column="0">
       <widget class="QLabel" name="nameLabel">
        <property name="text"><string>Name</string></property>
        <property name="buddy"><cstring>nameEdit</cstring></property>
       </widget>
      </item>
      <item row="0" column="1">
       <widget class="QLineEdit" name="nameEdit"/>
      </item>
      <item row="1" column="0">
       <widget class="QLabel" name="typeLabel">
        <property name="text"><string>Type</string></property>
        <property name="buddy"><cstring>typeComboBox</cstring></property>
       </widget>
      </item>
      <item row="1" column="1">
       <widget class="QComboBox" name="typeComboBox"/>
      </item>
      <item row="2" column="0">
       <widget class="QLabel" name="parentLabel">
        <property name="text"><string>Parent</string></property>
       </widget>
      </item>
      <item row="2" column="1">
       <layout class="QHBoxLayout" name="parentLayout">
        <item>
         <widget class="QLabel" name="parentDisplay">
          <property name="sizePolicy">
           <sizepolicy hsizetype="Expanding" vsizetype="Preferred">
            <horstretch>0</horstretch><verstretch>0</verstretch>
           </sizepolicy>
          </property>
          <property name="text"><string/></property>
         </widget>
        </item>
        <item>
         <widget class="QPushButton" name="parentAddButton">
          <property name="text"><string/></property>
          <property name="icon"><iconset theme="QIcon::ThemeIcon::ListAdd"/></property>
         </widget>
        </item>
        <item>
         <widget class="QPushButton" name="parentPickButton">
          <property name="text"><string/></property>
          <property name="icon"><iconset theme="QIcon::ThemeIcon::InsertLink"/></property>
         </widget>
        </item>
       </layout>
      </item>
      <item row="3" column="0">
       <widget class="QLabel" name="dateStartLabel">
        <property name="text"><string>Date (start)</string></property>
       </widget>
      </item>
      <item row="3" column="1">
       <layout class="QHBoxLayout" name="dateStartLayout">
        <item>
         <widget class="QLineEdit" name="dateStartEdit"/>
        </item>
        <item>
         <widget class="QPushButton" name="dateStartEditButton">
          <property name="text"><string/></property>
          <property name="icon"><iconset theme="edit-entry"/></property>
         </widget>
        </item>
       </layout>
      </item>
      <item row="4" column="0">
       <widget class="QLabel" name="dateEndLabel">
        <property name="text"><string>Date (end)</string></property>
       </widget>
      </item>
      <item row="4" column="1">
       <layout class="QHBoxLayout" name="dateEndLayout">
        <item>
         <widget class="QLineEdit" name="dateEndEdit"/>
        </item>
        <item>
         <widget class="QPushButton" name="dateEndEditButton">
          <property name="text"><string/></property>
          <property name="icon"><iconset theme="edit-entry"/></property>
         </widget>
        </item>
       </layout>
      </item>
      <item row="5" column="0">
       <widget class="QLabel" name="latitudeLabel">
        <property name="text"><string>Latitude</string></property>
        <property name="buddy"><cstring>latitudeSpinBox</cstring></property>
       </widget>
      </item>
      <item row="5" column="1">
       <widget class="QDoubleSpinBox" name="latitudeSpinBox">
        <property name="specialValueText"><string/></property>
        <property name="minimum"><double>-90.000000</double></property>
        <property name="maximum"><double>90.000000</double></property>
        <property name="singleStep"><double>0.000001</double></property>
        <property name="decimals"><number>6</number></property>
        <property name="value"><double>-200.000000</double></property>
       </widget>
      </item>
      <item row="6" column="0">
       <widget class="QLabel" name="longitudeLabel">
        <property name="text"><string>Longitude</string></property>
        <property name="buddy"><cstring>longitudeSpinBox</cstring></property>
       </widget>
      </item>
      <item row="6" column="1">
       <widget class="QDoubleSpinBox" name="longitudeSpinBox">
        <property name="specialValueText"><string/></property>
        <property name="minimum"><double>-180.000000</double></property>
        <property name="maximum"><double>180.000000</double></property>
        <property name="singleStep"><double>0.000001</double></property>
        <property name="decimals"><number>6</number></property>
        <property name="value"><double>-200.000000</double></property>
       </widget>
      </item>
      <item row="7" column="0">
       <widget class="QLabel" name="noteLabel">
        <property name="text"><string>Note</string></property>
        <property name="buddy"><cstring>noteEdit</cstring></property>
       </widget>
      </item>
      <item row="7" column="1">
       <layout class="QHBoxLayout" name="noteLayout">
        <item>
         <widget class="KRichTextEdit" name="noteEdit"/>
        </item>
        <item alignment="Qt::AlignmentFlag::AlignTop">
         <widget class="QPushButton" name="noteEditButton">
          <property name="text"><string/></property>
          <property name="icon"><iconset theme="edit-entry"/></property>
         </widget>
        </item>
       </layout>
      </item>
     </layout>
    </widget>
   </item>
   <item>
    <widget class="QDialogButtonBox" name="dialogButtons">
     <property name="standardButtons">
      <set>QDialogButtonBox::StandardButton::Cancel|QDialogButtonBox::StandardButton::Ok</set>
     </property>
    </widget>
   </item>
  </layout>
 </widget>
 <customwidgets>
  <customwidget>
   <class>KRichTextEdit</class>
   <extends>QTextEdit</extends>
   <header>KRichTextEdit</header>
  </customwidget>
 </customwidgets>
 <resources/>
 <connections>
  <connection>
   <sender>dialogButtons</sender><signal>accepted()</signal>
   <receiver>LocationEditorForm</receiver><slot>accept()</slot>
   <hints>
    <hint type="sourcelabel"><x>250</x><y>440</y></hint>
    <hint type="destinationlabel"><x>250</x><y>230</y></hint>
   </hints>
  </connection>
  <connection>
   <sender>dialogButtons</sender><signal>rejected()</signal>
   <receiver>LocationEditorForm</receiver><slot>reject()</slot>
   <hints>
    <hint type="sourcelabel"><x>250</x><y>440</y></hint>
    <hint type="destinationlabel"><x>250</x><y>230</y></hint>
   </hints>
  </connection>
 </connections>
</ui>
```

Note: latitude/longitude use `value = -200` as the sentinel for "not set" (outside valid range). `specialValueText` is empty — a real implementation should use a checkbox or a separate "clear" button, but for simplicity the sentinel value -200 is used and mapped to `std::nullopt` in the dialog code. An alternative is to use `QDoubleSpinBox` with `setSpecialValueText(i18n("Not set"))` and `minimum = -200`.

- [ ] **Step 2: Create `location_editor_dialog.h`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QDialog>
#include <optional>

namespace Ui {
class LocationEditorForm;
}

class LocationTypesListModel;

class LocationEditorDialog : public QDialog {
    Q_OBJECT

public:
    explicit LocationEditorDialog(std::optional<IntegerPrimaryKey> locationId,
                                  std::optional<IntegerPrimaryKey> initialParentId,
                                  QWidget* parent);

    static QVariant showDialogForNewLocation(std::optional<IntegerPrimaryKey> parentId, QWidget* parent);
    static void showDialogForExistingLocation(IntegerPrimaryKey locationId, QWidget* parent);

public Q_SLOTS:
    void accept() override;
    void editNoteWithEditor();
    void editDateStartWithEditor();
    void editDateEndWithEditor();
    void addNewLocationAsParent();
    void selectExistingLocationAsParent();

private:
    void updateParentDisplay() const;

    Ui::LocationEditorForm* form;
    std::optional<IntegerPrimaryKey> locationId;
    std::optional<IntegerPrimaryKey> parentId;
    LocationTypesListModel* typesModel;
};
```

- [ ] **Step 3: Create `location_editor_dialog.cpp`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "location_editor_dialog.h"

#include "dates/genealogical_date.h"
#include "dates/genealogical_date_editor_dialog.h"
#include "domain/location/location_repository.h"
#include "domain/location/location_types_list_model.h"
#include "link_existing/choose_existing_location_window.h"
#include "note_editor_dialog.h"
#include "ui_location_editor_dialog.h"
#include "utils/formatted_identifier_delegate.h"

#include <KLocalizedString>

using namespace Qt::StringLiterals;

LocationEditorDialog::LocationEditorDialog(
    std::optional<IntegerPrimaryKey> locationId,
    std::optional<IntegerPrimaryKey> initialParentId,
    QWidget* parent
) :
    QDialog(parent),
    form(new Ui::LocationEditorForm),
    locationId(locationId),
    parentId(initialParentId) {
    form->setupUi(this);

    connect(form->noteEditButton, &QPushButton::clicked, this, &LocationEditorDialog::editNoteWithEditor);
    connect(form->dateStartEditButton, &QPushButton::clicked, this, &LocationEditorDialog::editDateStartWithEditor);
    connect(form->dateEndEditButton, &QPushButton::clicked, this, &LocationEditorDialog::editDateEndWithEditor);
    connect(form->parentAddButton, &QPushButton::clicked, this, &LocationEditorDialog::addNewLocationAsParent);
    connect(form->parentPickButton, &QPushButton::clicked, this, &LocationEditorDialog::selectExistingLocationAsParent);

    typesModel = new LocationTypesListModel(this);
    form->typeComboBox->setModel(typesModel);
    form->typeComboBox->setModelColumn(LocationTypesListModel::TYPE);
    form->typeComboBox->setCurrentIndex(-1);

    // Use -200 as sentinel for "not set" (outside valid lat/lon range).
    form->latitudeSpinBox->setSpecialValueText(i18n("Not set"));
    form->latitudeSpinBox->setMinimum(-200.0);
    form->longitudeSpinBox->setSpecialValueText(i18n("Not set"));
    form->longitudeSpinBox->setMinimum(-200.0);

    form->noteEdit->enableRichTextMode();

    if (locationId.has_value()) {
        LocationRepository repo;
        if (const auto entity = repo.findById(*locationId)) {
            form->nameEdit->setText(entity->name);
            form->noteEdit->setTextOrHtml(entity->note);

            if (entity->typeId.has_value()) {
                auto idx = typesModel->match(
                    typesModel->index(0, LocationTypesListModel::ID), Qt::DisplayRole, *entity->typeId
                );
                if (!idx.isEmpty()) {
                    form->typeComboBox->setCurrentIndex(idx.constFirst().row());
                }
            }

            if (!entity->dateStart.isEmpty()) {
                form->dateStartEdit->setText(
                    GenealogicalDate::fromDatabaseRepresentation(entity->dateStart).toDisplayText()
                );
            }
            if (!entity->dateEnd.isEmpty()) {
                form->dateEndEdit->setText(
                    GenealogicalDate::fromDatabaseRepresentation(entity->dateEnd).toDisplayText()
                );
            }

            if (entity->latitude.has_value()) {
                form->latitudeSpinBox->setValue(*entity->latitude);
            }
            if (entity->longitude.has_value()) {
                form->longitudeSpinBox->setValue(*entity->longitude);
            }

            parentId = entity->parentId;
            setWindowTitle(i18n("Edit location"));
        }
    } else {
        setWindowTitle(i18n("Add new location"));
    }

    updateParentDisplay();
}

void LocationEditorDialog::accept() {
    auto name = form->nameEdit->text().trimmed();
    if (name.isEmpty()) {
        return;
    }

    auto typeRow = form->typeComboBox->currentIndex();
    std::optional<IntegerPrimaryKey> typeId;
    if (typeRow >= 0) {
        typeId = typesModel->index(typeRow, LocationTypesListModel::ID).data().toLongLong();
    }

    auto note = form->noteEdit->textOrHtml();

    auto latVal = form->latitudeSpinBox->value();
    auto lonVal = form->longitudeSpinBox->value();
    std::optional<double> latitude = latVal <= -200.0 ? std::nullopt : std::optional<double>(latVal);
    std::optional<double> longitude = lonVal <= -200.0 ? std::nullopt : std::optional<double>(lonVal);

    auto dateStartDisplay = form->dateStartEdit->text();
    auto dateStart = dateStartDisplay.isEmpty()
                         ? QString{}
                         : GenealogicalDate::fromDisplayText(dateStartDisplay).toDatabaseRepresentation();
    auto dateEndDisplay = form->dateEndEdit->text();
    auto dateEnd = dateEndDisplay.isEmpty()
                       ? QString{}
                       : GenealogicalDate::fromDisplayText(dateEndDisplay).toDatabaseRepresentation();

    LocationRepository repo;
    if (!locationId.has_value()) {
        auto newId = repo.insert(name, typeId, parentId);
        if (!newId) {
            return;
        }
        locationId = newId;
        // Update remaining fields.
        repo.update(*locationId, name, typeId, parentId, note, latitude, longitude, dateStart, dateEnd);
    } else {
        if (!repo.update(*locationId, name, typeId, parentId, note, latitude, longitude, dateStart, dateEnd)) {
            return;
        }
    }

    QDialog::accept();
}

QVariant LocationEditorDialog::showDialogForNewLocation(
    std::optional<IntegerPrimaryKey> parentId, QWidget* parent
) {
    auto* dialog = new LocationEditorDialog(std::nullopt, parentId, parent);
    if (dialog->exec() != Accepted || !dialog->locationId) {
        return {};
    }
    return QVariant::fromValue(*dialog->locationId);
}

void LocationEditorDialog::showDialogForExistingLocation(IntegerPrimaryKey locationId, QWidget* parent) {
    auto* dialog = new LocationEditorDialog(locationId, std::nullopt, parent);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void LocationEditorDialog::updateParentDisplay() const {
    if (parentId.has_value()) {
        form->parentDisplay->setText(
            format_id(FormattedIdentifierDelegate::LOCATION, *parentId)
        );
    } else {
        form->parentDisplay->clear();
    }
}

void LocationEditorDialog::editNoteWithEditor() {
    auto current = form->noteEdit->textOrHtml();
    if (auto note = NoteEditorDialog::editText(current, i18n("Edit note"), this); !note.isEmpty()) {
        form->noteEdit->setTextOrHtml(note);
    }
}

void LocationEditorDialog::editDateStartWithEditor() {
    auto current = form->dateStartEdit->text();
    auto startDate = GenealogicalDate::fromDisplayText(current);
    if (auto date = GenealogicalDateEditorDialog::editDate(startDate, this); date.isValid()) {
        form->dateStartEdit->setText(date.toDisplayText());
    }
}

void LocationEditorDialog::editDateEndWithEditor() {
    auto current = form->dateEndEdit->text();
    auto endDate = GenealogicalDate::fromDisplayText(current);
    if (auto date = GenealogicalDateEditorDialog::editDate(endDate, this); date.isValid()) {
        form->dateEndEdit->setText(date.toDisplayText());
    }
}

void LocationEditorDialog::addNewLocationAsParent() {
    auto newId = showDialogForNewLocation(std::nullopt, this);
    if (newId.isValid()) {
        parentId = newId.toLongLong();
        updateParentDisplay();
    }
}

void LocationEditorDialog::selectExistingLocationAsParent() {
    auto selected = ChooseExistingLocationWindow::selectLocation(this);
    if (selected.isValid()) {
        parentId = selected.toLongLong();
        updateParentDisplay();
    }
}
```

- [ ] **Step 4: Register in `src/CMakeLists.txt`**

```cmake
  editors/location_editor_dialog.h
  editors/location_editor_dialog.cpp
```

- [ ] **Step 5: Build**

```bash
cd build && cmake .. && make -j$(nproc) 2>&1 | grep "error:" | head -20
```

Expected: clean build.

- [ ] **Step 6: Commit**

```bash
git add src/editors/location_editor_dialog.ui \
        src/editors/location_editor_dialog.h \
        src/editors/location_editor_dialog.cpp \
        src/CMakeLists.txt
git commit -m "feat: add LocationEditorDialog"
```

---

## Task 7: LocationManagementWindow

**Files:**
- Create: `src/lists/location_management_window.h/cpp`
- Modify: `src/CMakeLists.txt`

- [ ] **Step 1: Create `location_management_window.h`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QMainWindow>

class QAction;
class QTreeView;

class LocationManagementWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit LocationManagementWindow(QWidget* parent = nullptr);

private Q_SLOTS:
    void addRootLocation();
    void addChildLocation();
    void editSelected();
    void deleteSelected();
    void onSelectionChanged();

private:
    [[nodiscard]] std::optional<IntegerPrimaryKey> selectedLocationId() const;

    QTreeView* treeView;
    QAction* addChildAction;
    QAction* editAction;
    QAction* deleteAction;
};
```

- [ ] **Step 2: Create `location_management_window.cpp`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "location_management_window.h"

#include "domain/location/location_list_model.h"
#include "domain/location/location_repository.h"
#include "editors/location_editor_dialog.h"
#include "utils/model_utils.h"
#include "utils/tree_proxy_model.h"

#include <KLocalizedString>
#include <QAction>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

using namespace Qt::StringLiterals;

LocationManagementWindow::LocationManagementWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(i18n("Manage locations"));

    auto* locationsModel = new LocationListModel(this);

    auto* treeModel = new TreeProxyModel(this);
    treeModel->setSourceModel(locationsModel);
    treeModel->setIdColumn(LocationListModel::ID);
    treeModel->setParentIdColumn(LocationListModel::PARENT_ID);

    auto* filtered = new QSortFilterProxyModel(this);
    filtered->setSourceModel(treeModel);
    filtered->setFilterKeyColumn(LocationListModel::NAME);
    filtered->setFilterCaseSensitivity(Qt::CaseInsensitive);
    filtered->setRecursiveFilteringEnabled(true);

    treeView = new QTreeView(this);
    treeView->setModel(filtered);
    treeView->setSelectionBehavior(QTreeView::SelectRows);
    treeView->setSelectionMode(QTreeView::SingleSelection);
    treeView->setSortingEnabled(true);
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    treeView->header()->setSectionResizeMode(LocationListModel::NAME, QHeaderView::Stretch);
    treeView->hideColumn(LocationListModel::PARENT_ID);
    treeView->hideColumn(LocationListModel::TYPE_ID);
    treeView->expandToDepth(1);
    treeView->setUniformRowHeights(true);

    auto* searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText(i18n("Search…"));
    searchBox->setClearButtonEnabled(true);
    connect(searchBox, &QLineEdit::textEdited, filtered, &QSortFilterProxyModel::setFilterFixedString);

    auto* toolbar = addToolBar(i18n("Actions"));

    auto* addRootAction = new QAction(QIcon::fromTheme(u"list-add"_s), i18n("Add root location"), this);
    connect(addRootAction, &QAction::triggered, this, &LocationManagementWindow::addRootLocation);
    toolbar->addAction(addRootAction);

    addChildAction = new QAction(QIcon::fromTheme(u"list-add"_s), i18n("Add child location"), this);
    addChildAction->setEnabled(false);
    connect(addChildAction, &QAction::triggered, this, &LocationManagementWindow::addChildLocation);
    toolbar->addAction(addChildAction);

    editAction = new QAction(QIcon::fromTheme(u"document-edit"_s), i18n("Edit"), this);
    editAction->setEnabled(false);
    connect(editAction, &QAction::triggered, this, &LocationManagementWindow::editSelected);
    toolbar->addAction(editAction);

    deleteAction = new QAction(QIcon::fromTheme(u"edit-delete"_s), i18n("Delete"), this);
    deleteAction->setEnabled(false);
    connect(deleteAction, &QAction::triggered, this, &LocationManagementWindow::deleteSelected);
    toolbar->addAction(deleteAction);

    connect(
        treeView->selectionModel(),
        &QItemSelectionModel::selectionChanged,
        this,
        &LocationManagementWindow::onSelectionChanged
    );

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    layout->addWidget(searchBox);
    layout->addWidget(treeView);
    setCentralWidget(central);
}

std::optional<IntegerPrimaryKey> LocationManagementWindow::selectedLocationId() const {
    const auto selected = treeView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        return std::nullopt;
    }
    const auto id = selected.constFirst().siblingAtColumn(LocationListModel::ID).data(Qt::EditRole);
    if (!id.isValid()) {
        return std::nullopt;
    }
    return id.value<IntegerPrimaryKey>();
}

void LocationManagementWindow::onSelectionChanged() {
    const bool hasSelection = selectedLocationId().has_value();
    addChildAction->setEnabled(hasSelection);
    editAction->setEnabled(hasSelection);
    deleteAction->setEnabled(hasSelection);
}

void LocationManagementWindow::addRootLocation() {
    LocationEditorDialog::showDialogForNewLocation(std::nullopt, this);
}

void LocationManagementWindow::addChildLocation() {
    auto parentId = selectedLocationId();
    LocationEditorDialog::showDialogForNewLocation(parentId, this);
}

void LocationManagementWindow::editSelected() {
    auto id = selectedLocationId();
    if (!id.has_value()) {
        return;
    }
    LocationEditorDialog::showDialogForExistingLocation(*id, this);
}

void LocationManagementWindow::deleteSelected() {
    auto id = selectedLocationId();
    if (!id.has_value()) {
        return;
    }

    LocationRepository repo;
    if (repo.isUsed(*id)) {
        QMessageBox::warning(
            this,
            i18n("Cannot delete location"),
            i18n("This location is assigned to one or more events and cannot be deleted.")
        );
        return;
    }

    if (QMessageBox::question(
            this, i18n("Delete location"), i18n("Are you sure you want to delete this location?")
        ) == QMessageBox::Yes) {
        repo.deleteLocation(*id);
    }
}
```

- [ ] **Step 3: Register in `src/CMakeLists.txt`**

```cmake
  lists/location_management_window.h
  lists/location_management_window.cpp
```

- [ ] **Step 4: Build**

```bash
cd build && cmake .. && make -j$(nproc) 2>&1 | grep "error:" | head -20
```

Expected: clean build.

- [ ] **Step 5: Commit**

```bash
git add src/lists/location_management_window.h \
        src/lists/location_management_window.cpp \
        src/CMakeLists.txt
git commit -m "feat: add LocationManagementWindow with tree CRUD"
```

---

## Task 8: Event editor integration

**Files:**
- Modify: `src/domain/event/event_entities.h`
- Modify: `src/domain/event/event_repository.h`
- Modify: `src/domain/event/event_repository.cpp`
- Modify: `autotests/event_repository_test.cpp`
- Modify: `src/editors/event_editor_dialog.ui`
- Modify: `src/editors/event_editor_dialog.h`
- Modify: `src/editors/event_editor_dialog.cpp`

- [ ] **Step 1: Write failing test for updated `updateEvent`**

In `autotests/event_repository_test.cpp`, add a new test in `private Q_SLOTS`:

```cpp
void testUpdateEventWithLocation() {
    auto typeId = insertEventType();
    auto locId = insertQuery(u"INSERT INTO locations (name) VALUES ('Amsterdam')"_s);

    EventRepository repo;
    auto id = repo.insertEvent(typeId);
    QVERIFY(id.has_value());

    bool ok = repo.updateEvent(*id, typeId, u"1900"_s, u"Test"_s, u""_s, locId);
    QVERIFY(ok);

    auto result = repo.findEventById(*id);
    QVERIFY(result.has_value());
    QVERIFY(result->locationId.has_value());
    QCOMPARE(*result->locationId, locId);
}

void testUpdateEventClearsLocation() {
    auto typeId = insertEventType();
    auto locId = insertQuery(u"INSERT INTO locations (name) VALUES ('Amsterdam')"_s);

    EventRepository repo;
    auto id = repo.insertEvent(typeId);
    QVERIFY(id.has_value());

    repo.updateEvent(*id, typeId, QString{}, QString{}, QString{}, locId);

    bool ok = repo.updateEvent(*id, typeId, QString{}, QString{}, QString{}, std::nullopt);
    QVERIFY(ok);

    auto result = repo.findEventById(*id);
    QVERIFY(result.has_value());
    QVERIFY(!result->locationId.has_value());
}
```

Also update the existing `testUpdateEvent` call site — the existing `updateEvent` call has no locationId argument; after the signature change it needs `std::nullopt` added:

```cpp
bool ok = repo.updateEvent(*id, typeId, u"1900-01-01"_s, u"Test event"_s, u"A note"_s, std::nullopt);
```

- [ ] **Step 2: Run tests to confirm they fail**

```bash
cd build && make -j$(nproc) 2>&1 | grep "error:" | head -10
```

Expected: compile errors — `locationId` doesn't exist on `EventEntity` yet.

- [ ] **Step 3: Update `EventEntity` in `event_entities.h`**

In `src/domain/event/event_entities.h`, change the `EventEntity` struct:

```cpp
struct EventEntity {
    IntegerPrimaryKey id = -1;
    IntegerPrimaryKey typeId = -1;
    QString date;
    QString name;
    QString note;
    std::optional<IntegerPrimaryKey> locationId;

    static EventEntity fromSql(const QSqlQuery& query) {
        return {
            .id = query.value(u"id").toLongLong(),
            .typeId = query.value(u"type_id").toLongLong(),
            .date = query.value(u"date").toString(),
            .name = query.value(u"name").toString(),
            .note = query.value(u"note").toString(),
            .locationId = validOrOptional<IntegerPrimaryKey>(query.value(u"location_id")),
        };
    }
};
```

Add `#include "core/query_utils.h"` to the includes at the top of `event_entities.h` if not already present.

- [ ] **Step 4: Update `EventRepository` header**

In `src/domain/event/event_repository.h`, change `updateEvent` and `insertFullEvent`:

```cpp
bool updateEvent(
    IntegerPrimaryKey id,
    IntegerPrimaryKey typeId,
    const QString& date,
    const QString& name,
    const QString& note,
    std::optional<IntegerPrimaryKey> locationId
) const;

std::optional<IntegerPrimaryKey> insertFullEvent(
    IntegerPrimaryKey typeId,
    const QString& date,
    const QString& name,
    const QString& note,
    IntegerPrimaryKey personId,
    IntegerPrimaryKey roleId,
    std::optional<IntegerPrimaryKey> locationId = std::nullopt
) const;
```

Add `#include <optional>` if not already present.

- [ ] **Step 5: Update `EventRepository` implementation**

In `src/domain/event/event_repository.cpp`, find `updateEvent` and update the SQL and bindings:

```cpp
bool EventRepository::updateEvent(
    IntegerPrimaryKey id,
    IntegerPrimaryKey typeId,
    const QString& date,
    const QString& name,
    const QString& note,
    std::optional<IntegerPrimaryKey> locationId
) const {
    return QueryHelper::executeAndNotify<Schema::Events>(
        id,
        u"UPDATE events SET type_id = :type_id, date = :date, name = :name, note = :note, "
        u"location_id = :location_id WHERE id = :id"_s,
        {
            {u":type_id"_s, typeId},
            {u":date"_s, date},
            {u":name"_s, name},
            {u":note"_s, note},
            {u":location_id"_s, locationId.has_value() ? QVariant(*locationId) : QVariant{}},
            {u":id"_s, id},
        }
    );
}
```

Find `insertFullEvent` and add `locationId` support. The method inserts the event and then immediately updates it with remaining fields including locationId. Pass `locationId` through to the `insertEvent` → `updateEvent` chain, or inline it. The simplest approach — change the INSERT to include `location_id`:

```cpp
std::optional<IntegerPrimaryKey> EventRepository::insertFullEvent(
    IntegerPrimaryKey typeId,
    const QString& date,
    const QString& name,
    const QString& note,
    IntegerPrimaryKey personId,
    IntegerPrimaryKey roleId,
    std::optional<IntegerPrimaryKey> locationId
) const {
    auto newEventId = QueryHelper::insert(
        u"INSERT INTO events (type_id, date, name, note, location_id) "
        u"VALUES (:type_id, :date, :name, :note, :location_id)"_s,
        {
            {u":type_id"_s, typeId},
            {u":date"_s, date},
            {u":name"_s, name},
            {u":note"_s, note},
            {u":location_id"_s, locationId.has_value() ? QVariant(*locationId) : QVariant{}},
        }
    );
    if (!newEventId) {
        return std::nullopt;
    }
    DataEventBroker::instance().notifyChanged<Schema::Events>(*newEventId);

    auto relationId = insertEventRelation(*newEventId, personId, roleId);
    if (!relationId) {
        return std::nullopt;
    }
    return newEventId;
}
```

Also find the existing `insertEvent` helper (if it exists separately) and check if it still compiles; it does not need to change since it doesn't set `location_id`.

- [ ] **Step 6: Fix existing test call sites for `updateEvent`**

In `autotests/event_repository_test.cpp`, add `std::nullopt` to all existing `repo.updateEvent(...)` calls that don't already have a `locationId` argument. Search for `updateEvent` in the file and add `, std::nullopt` before the closing `)`.

- [ ] **Step 7: Build and run event repository tests**

```bash
cd build && cmake .. && make -j$(nproc) && ctest -R event_repository_test -V 2>&1 | tail -30
```

Expected: all tests PASS including the two new location tests.

- [ ] **Step 8: Add location row to `event_editor_dialog.ui`**

In `src/editors/event_editor_dialog.ui`, add a new row (row 4, pushing the description row to row 5... actually insert before row 3 "Description" by renumbering, OR just add as row 4 after Name at row 2):

Inside `formLayout_2` (the "Event information" group), after the `eventNameEdit` item at row 2, add:

```xml
<item row="3" column="0">
 <widget class="QLabel" name="locationLabel">
  <property name="text">
   <string>Location</string>
  </property>
  <property name="buddy">
   <cstring>locationComboBox</cstring>
  </property>
 </widget>
</item>
<item row="3" column="1">
 <layout class="QHBoxLayout" name="locationLayout">
  <item>
   <widget class="QComboBox" name="locationComboBox"/>
  </item>
  <item>
   <widget class="QPushButton" name="locationNewButton">
    <property name="text">
     <string/>
    </property>
    <property name="icon">
     <iconset theme="QIcon::ThemeIcon::ListAdd"/>
    </property>
   </widget>
  </item>
 </layout>
</item>
```

Renumber the existing Description row from 3 to 4:

```xml
<item row="4" column="0">
 <widget class="QLabel" name="label">
  <property name="text"><string>Description</string></property>
 </widget>
</item>
<item row="4" column="1">
 ...existing horizontalLayout2 content...
</item>
```

- [ ] **Step 9: Update `event_editor_dialog.h`**

Add to the private members of `EventEditorDialog`:

```cpp
class LocationPathsModel;
// (in the forward declarations at the top of the file)
```

And in the class body, add:

```cpp
LocationPathsModel* locationsModel;
```

- [ ] **Step 10: Update `event_editor_dialog.cpp`**

Add include:
```cpp
#include "domain/location/location_paths_model.h"
#include "editors/location_editor_dialog.h"
```

In `EventEditorDialog::setupUi()`, add after the existing model setup:

```cpp
locationsModel = new LocationPathsModel(this);
form->locationComboBox->setModel(locationsModel);
form->locationComboBox->setModelColumn(LocationPathsModel::FULL_PATH);
form->locationComboBox->setCurrentIndex(0); // index 0 = sentinel empty entry

connect(form->locationNewButton, &QPushButton::clicked, this, [this]() {
    auto newId = LocationEditorDialog::showDialogForNewLocation(std::nullopt, this);
    if (newId.isValid()) {
        // After the model reloads, find the new location and select it.
        auto id = newId.toLongLong();
        auto matches = locationsModel->match(
            locationsModel->index(0, LocationPathsModel::ID), Qt::EditRole, id
        );
        if (!matches.isEmpty()) {
            form->locationComboBox->setCurrentIndex(matches.constFirst().row());
        }
    }
});
```

In the existing-event constructor block (after `if (const auto event = repo.findEventById(eventId))`), add location pre-selection:

```cpp
if (event->locationId.has_value()) {
    auto matches = locationsModel->match(
        locationsModel->index(0, LocationPathsModel::ID),
        Qt::EditRole,
        QVariant::fromValue(*event->locationId)
    );
    if (!matches.isEmpty()) {
        form->locationComboBox->setCurrentIndex(matches.constFirst().row());
    }
}
```

In `EventEditorDialog::accept()`, collect the location ID before the transaction block:

```cpp
auto locationRow = form->locationComboBox->currentIndex();
std::optional<IntegerPrimaryKey> locationId;
if (locationRow > 0) { // row 0 is the empty sentinel
    auto id = locationsModel->index(locationRow, LocationPathsModel::ID).data(Qt::EditRole).toLongLong();
    if (id > 0) {
        locationId = id;
    }
}
```

Pass `locationId` to `repo.updateEvent(...)` and `repo.insertFullEvent(...)` — add it as the last argument in both calls.

- [ ] **Step 11: Build and run all tests**

```bash
cd build && cmake .. && make -j$(nproc) && ctest -V 2>&1 | tail -50
```

Expected: all tests PASS. Zero regressions.

- [ ] **Step 12: Commit**

```bash
git add src/domain/event/event_entities.h \
        src/domain/event/event_repository.h \
        src/domain/event/event_repository.cpp \
        src/editors/event_editor_dialog.h \
        src/editors/event_editor_dialog.cpp \
        src/editors/event_editor_dialog.ui \
        autotests/event_repository_test.cpp
git commit -m "feat: wire location into event editor and event repository"
```

---

## Self-Review Checklist

- [x] **Schema**: `location_types` + `locations` + `events.location_id` covered in Task 1
- [x] **Migration**: `002_add_locations.sql` + migration registration + migration tests
- [x] **Builtin types**: `init.sql` entries in Task 1
- [x] **Schema tags**: `Schema::LocationTypes` + `Schema::Locations` in Task 1
- [x] **Entities**: `LocationTypeEntity`, `LocationEntity`, `LocationDisplayEntity` in Task 2
- [x] **Repository**: all methods including recursive CTE, `findOrCreate`, `isUsed`, `isLocationTypeUsed`
- [x] **Models**: `LocationTypesListModel`, `LocationListModel`, `LocationPathsModel` with sentinel row
- [x] **LocationTypesManagementWindow**: Task 4
- [x] **ChooseExistingLocationWindow**: Task 5, uses `LocationPathsModel`
- [x] **LOCATION identifier**: Task 5
- [x] **LocationEditorDialog**: Task 6 — name, type combobox, parent picker (label + two buttons), dates, lat/lon, note
- [x] **LocationManagementWindow**: Task 7 — tree view, Add Root / Add Child / Edit / Delete
- [x] **Event entity `locationId`**: Task 8
- [x] **`updateEvent` + `insertFullEvent` signatures**: Task 8
- [x] **Event editor UI location row**: Task 8
- [x] **Event editor wiring**: combobox backed by `LocationPathsModel`, inline creation, pre-selection on load
- [x] **Test coverage**: migration tests, repository tests, updated event repository tests
- [x] **CMakeLists.txt**: all new files registered, migration resource added
