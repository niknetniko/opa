# Architecture

How the code is structured. Describes the code as it is; planned changes are in [status.md](status.md).

```
View        Widgets, dialogs, docks         src/main, src/editors, src/docks, src/ui, src/person_detail
Model       ObjectTableModel<T>, read-only  src/model, src/domain/*/*_model.*
Repository  all SQL, returns entities       src/domain/*/*_repository.*
Database    SQLite via QSqlQuery            src/database, src/core
```

## Entities

Plain structs in `src/domain/*/*_entities.h`, no `QObject`. Each has `static T fromSql(const QSqlQuery&)`.
Nullable columns are `std::optional` (see [database_design.md](database_design.md)).

## Repositories

All SQL lives here. Inherit `BaseRepository` (`src/core/base_repository.h`).

- Read: `fetchAll<T>(sql, bindings)` and `fetchOne<T>(sql, bindings)`. Return `QList<T>` or `std::optional<T>`,
  never a Qt model.
- Dynamic filtering/sorting: `QueryHelper::SqlQueryBuilder` (`src/core/query_helper.h`). Complex queries: raw SQL
  with CTEs.
- Write: after a successful INSERT/UPDATE/DELETE, call
  `DataEventBroker::instance().notifyChanged<Schema::Table>(id)`, or use `QueryHelper::executeAndNotify<T>`.
- Multiple writes: wrap in `executeInTransaction(...)` (`src/database/database.h`). Notifications are batched and
  only sent if the transaction commits.
- Repositories use the main thread's connection, so call them from the main thread only.

## Models

Use `ObjectTableModel<T>` (`src/model/object_table_model.h`); don't write custom `QAbstractTableModel`s.
Columns are extractor lambdas or member pointers:

```cpp
setColumn(DATE, i18n("Date"), [](const PersonEventEntity& e) -> QVariant { ... });
setColumn(NAME, i18n("Name"), &PersonEventEntity::name);
connectToTable<Schema::Events>(this);  // calls reload() on changes
```

`reload()` re-runs the repository query and calls `setItems`. See `domain/event/person_events_model.cpp`.
Small lookup lists (types, roles) use `DistinctStringModel`.

## Change notifications

`DataEventBroker` (`src/core/data_event_broker.h`) emits `entityChanged(table, id)`. Listen with
`connectToTable<Schema::Table>(receiver, ...)`: with no callback it calls `receiver->reload()`, with an id it only
fires for that row (or for table-wide changes). Table tags live in `src/database/schema.h`.

## Editing

Models are read-only; all writes go through repositories. Never write SQL in UI code.

- **Form-based (preferred):** the dialog reads its widgets, builds or updates an entity, calls the repository.
  See `LocationEditorDialog::accept()`.
- **Inline:** `ObjectTableModel::setColumn(index, header, extractor, setter)`; the setter lambda updates the row
  and calls the repository, which notifies the broker.

## Legacy

`QSqlTableModel`, `QDataWidgetMapper` and `AbstractEditorDialog` code is being removed (list in status.md).
Don't use it for new code.
