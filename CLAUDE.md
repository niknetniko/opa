## Project Overview

**Opa** is an early-stage genealogy desktop application written in C++20 using Qt 6 and KDE Frameworks 6.
It uses SQLite for storage.

Development dependencies are managed via the included Nix flake (`flake.nix`). 
Run `nix develop` for a reproducible shell with all dependencies.

Code formatting uses `.clang-format` (LLVM style). Static analysis config is in `.clang-tidy`.

## Code

Opa must use modern Qt and C++ features.
For example, avoid raw pointers and use smart pointers instead.
Prefer C++20 features like `std::optional` and `std::variant`.
Use Qt's modern signal-slot syntax with lambdas and `auto` for type inference.

Do not use Hungarian notation.
Do not put comments on closing braces, like classes or namespaces.

If multiple strings are used in a file, use the shorthand Qt string literals: `u"string"_s`.

Opa must use built-in functionality of KDE where possible and integrate with KDE Frameworks.

## Architecture

Read `ai/architecture.md` before making changes. It is the authoritative source for patterns and migration checklists.

### Layer Stack

```
View Layer         (Qt Widgets, Dialogs, Docks — src/main/, src/editors/, src/docks/, src/ui/)
     ↓
Model/ViewModel    (ObjectTableModel<T> — src/model/)
     ↓
Repository Layer   (SQL execution, returns typed entities — src/domain/*/  e.g. PersonRepository)
     ↓
Database Layer     (SQLite via QSqlQuery — src/database/)
```

### Key Patterns

**Entities** — Plain C++ structs (no `QObject`) with `static T fromSql(const QSqlQuery&)` factory. Stored in `QList<T>` or `std::vector<T>`. Defined in `*_entities.h` per domain.

**Repositories** — All SQL lives here. Inherit `BaseRepository` (`src/core/base_repository.h`). Use `fetchAll<T>()` / `fetchOne<T>()` helpers. Never return `QAbstractItemModel`. Emit `DataEventBroker` notifications on write.

**ObjectTableModel<T>** (`src/model/object_table_model.h`) — Generic read-only model for all entities. Columns defined with extractor lambdas. Do not write custom `QAbstractTableModel` subclasses.

**DataEventBroker** (`src/core/data_event_broker.h`) — Singleton signal bus. Repositories call `notifyChanged<Schema::Table>(id)` after INSERT/UPDATE. Views/models connect with `connectToTable<Schema::Table>(this, [this]{ reload(); })`.

**QueryHelper** (`src/core/query_helper.h`) — Dynamic SQL builder for WHERE/ORDER BY/LIMIT. Use for filterable/sortable lists. Use raw SQL with CTEs for complex queries.

### Editing Approaches (CQRS)

Models are **read-only**. All writes go through repositories.

- **Form-based (preferred)**: Dialog collects values → mutates entity struct → calls `repository.updateEntity(entity)`.
- **Inline**: `ObjectTableModel` setter lambda → calls repository → broker notifies refresh.

Never write SQL in UI code. Never modify the DB from a model directly.

## Database Schema

SQLite. Key tables: `people`, `names`, `name_origins`, `events`, `event_types`, `event_roles`, `event_relations` (many-to-many), `sources`, `event_sources`, `name_sources`, `person_sources`.
Schema defined in `src/database/schema.sql`. Initialized via `init.sql` + `seed.sql` (built-in types/roles/origins).
Data in the database should be highly normalized.

Logging categories: `opa` (general), `opa.sql` (SQL queries).
