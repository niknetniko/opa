# Database Design Guidelines

## Nullability

**Never use `NOT NULL DEFAULT ''` for optional text columns.**

If a column is conceptually optional (a title, note, description, etc.), declare it as `TEXT` (nullable). Empty strings are not a substitute for NULL — they obscure the difference between "not set" and "set to empty", and they cause `NOT NULL` constraint failures when application code correctly passes `NULL` for an absent value.

```sql
-- Wrong
note TEXT NOT NULL DEFAULT ''

-- Correct
note TEXT
```

Only apply `NOT NULL` to columns that are genuinely mandatory. Never add `DEFAULT ''`.

## Optional columns in C++

Nullable text columns must be represented as `std::optional<QString>` in entity structs, not plain `QString`. Use null-aware `fromSql` factories:

```cpp
auto note = query.value(u"note");
.note = note.isNull() ? std::nullopt : std::make_optional(note.toString()),
```

When binding optionals back to SQL, pass `QVariant{}` (which binds as SQL NULL) for `std::nullopt`:

```cpp
{u":note"_s, note ? QVariant(*note) : QVariant{}}
```

## Normalization

Data in the database should be highly normalized. Avoid storing derived or redundant data.

## Migrations

- Migration SQL files cannot contain `--` comments (the migration runner does not support them).
- Each migration file is registered in the `constexpr std::array migrations` in `src/database/database.cpp`.
- New databases are stamped with the version of the latest migration (`migrations.back().version`).
- Migrations are immutable once shipped; schema changes after that require a new numbered migration.

## Schema files

- `src/database/schema.sql` — defines the full schema for new databases.
- `src/database/init.sql` — run after schema creation (e.g. enabling foreign keys).
- `src/database/seed.sql` — inserts built-in types, roles, and origins.
- `src/database/migrations/NNN_description.sql` — incremental migrations.
- Table name constants live in `src/database/schema.h` as `Schema::FooTable` / `Schema::Foo` tag structs.
