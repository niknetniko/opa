# Design: Type Translations

## Context

Event types and location types in Opa are currently stored as plain English strings in the database. Event types have a partial workaround (C++ enum + `kli18n()` map), but location types have no translation support at all. Neither supports user-defined type translations. This design introduces a unified translation system: built-in types use KDE `.po` files resolved at display time; user-defined types optionally store translations in the database via a new `type_translations` table.

The goal is that all type names display in the user's system language (via KDE locale), with no in-app language setting needed. User-defined types show their stored name by default, with optional per-locale overrides manageable via the UI.

---

## Schema Changes

**Migration:** `src/database/migrations/003_add_type_translations.sql`

```sql
CREATE TABLE event_type_translations (
    id       INTEGER PRIMARY KEY AUTOINCREMENT,
    type_id  INTEGER NOT NULL REFERENCES event_types(id) ON DELETE CASCADE,
    locale   TEXT NOT NULL,
    name     TEXT NOT NULL,
    UNIQUE(type_id, locale)
);

CREATE TABLE location_type_translations (
    id       INTEGER PRIMARY KEY AUTOINCREMENT,
    type_id  INTEGER NOT NULL REFERENCES location_types(id) ON DELETE CASCADE,
    locale   TEXT NOT NULL,
    name     TEXT NOT NULL,
    UNIQUE(type_id, locale)
);
```

The existing `type` column on `event_types`/`location_types` remains as the canonical stored name and fallback.

Update `src/database/schema.h` and `src/database/schema.sql` accordingly.

---

## Built-in Type Translation

Built-in types are translated at display time via `kli18n()` — never stored in the DB. No UI is shown for built-in type translations.

**Location types** need the same treatment that event types already have:
- Add a `LocationTypes` enum + `kli18n()` map in `src/domain/location/location_entities.h` for the 7 built-in types: Country, Province, County, City, Village, Parish, Address
- Wire a translator function into `LocationTypesListModel` (matching the existing `EventTypesListModel` pattern)
- Add location type strings to `.po` extraction via `Messages.sh`

---

## Display Resolution Logic

New utility: `src/utils/type_translation_resolver.h/.cpp`

Resolution order for any type entity:
1. If `entity.builtin == true` → use domain's `kli18n()` map (e.g. `EventTypes::toDisplayString`, `LocationTypes::toDisplayString`)
2. If `entity.builtin == false` → query `type_translations` for `(type_id, current_locale)`
3. If not found → try language-only fallback (`"nl_BE"` → `"nl"`)
4. If still not found → return `entity.type` (stored name)

Current locale comes from `QLocale::system().name()`. No in-app language setting.

This resolver is used by:
- `BuiltinTextTranslatingDelegate` (already wired into type list views)
- Type combo boxes in editors (event editor, location editor)
- Future report generation

---

## User Type Translation UI

A **"Translations" button/action** in `EventTypesManagementWindow` and `LocationTypesManagementWindow`, enabled only when a non-built-in type row is selected.

Opens `TypeTranslationsDialog` (`src/editors/type_translations_dialog.h/.cpp/.ui`):
- Table showing existing translations (Language, Name)
- Language column shows human-readable name + code (e.g. "Dutch (nl)")
- **Add**: `KLanguageButton` (from `KF6::WidgetsAddons`, already a dependency) to select locale + text field for name
- **Remove**: deletes selected row
- **Close**: dismisses dialog
- Changes notify via `DataEventBroker` so open combo boxes refresh

`TypeTranslationsDialog` is parameterised on the repository to work for both event and location types.

---

## Repository & Code Structure

### New files
| File | Purpose |
|------|---------|
| `src/domain/event/event_type_translation_entities.h` | `EventTypeTranslationEntity { id, typeId, locale, name }` |
| `src/domain/event/event_type_translation_repository.h/.cpp` | `fetchAll(typeId)`, `insert`, `remove` |
| `src/domain/location/location_type_translation_entities.h` | `LocationTypeTranslationEntity { id, typeId, locale, name }` |
| `src/domain/location/location_type_translation_repository.h/.cpp` | `fetchAll(typeId)`, `insert`, `remove` |
| `src/editors/type_translations_dialog.h/.cpp/.ui` | Shared translations management dialog |
| `src/utils/type_translation_resolver.h/.cpp` | Display resolution logic |
| `src/database/migrations/003_add_type_translations.sql` | Schema migration |

### Modified files
| File | Change |
|------|--------|
| `src/domain/location/location_entities.h` | Add `LocationTypes` enum + `kli18n()` map |
| `src/utils/builtin_text_translating_delegate.cpp` | Use resolver for non-builtin types |
| `src/lists/location_types_management_window.*` | Add Translations button, wire translator function |
| `src/lists/event_types_management_window.*` | Add Translations button |
| `src/database/schema.h` | Add new table constants |
| `src/database/schema.sql` | Add new table definitions |
| `src/CMakeLists.txt` | Add new source files |

---

## Verification

1. **Build**: `cmake --build build` with no warnings on new files
2. **Location types translated**: Open the app, check that built-in location types (Country, Province, etc.) display in the system language (requires a `.po` file with translations for that language)
3. **User type fallback**: Create a custom event type "Adoptie" — it should display as "Adoptie" with no translation set
4. **User type translation**: Open Translations dialog for "Adoptie", add `en` → "Adoption". Switch system locale to English, reopen — "Adoption" should display
5. **Built-in type no translation UI**: Select a built-in type — Translations button should be disabled/hidden
6. **Cascade delete**: Delete a user type — its translation rows should be removed (`ON DELETE CASCADE`)
7. **Autotests**: Add tests in `autotests/` for `TypeTranslationResolver` covering all fallback cases
