# Locations Feature Design

**Date:** 2026-04-13
**Status:** Approved

## Context

Opa currently has no concept of place. Events (births, marriages, deaths) happen somewhere, but that information cannot be recorded. This feature adds a normalized `locations` table and attaches locations to events, following GEDCOM conventions (one location per event).

## Requirements

- Locations are hierarchical with unlimited depth (e.g. city → province → country)
- Each location stores: name, optional parent, optional type (City/Country/Parish/…), note (rich text), lat/long coordinates, date range (start/end)
- Location types are a normalized lookup table with builtin entries (like `event_types`)
- Locations are attached to events only (one nullable FK per event)
- Managed via a dedicated window; also creatable inline from the event editor

---

## Schema

### New table: `location_types`

```sql
CREATE TABLE location_types (
  id      INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type    TEXT NOT NULL,
  builtin BOOLEAN NOT NULL DEFAULT FALSE
);
```

Builtin types added to `src/database/init.sql`: Country, Province/State, County/District, City/Town, Village, Parish, Street/Address.

### New table: `locations`

```sql
CREATE TABLE locations (
  id         INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  name       TEXT NOT NULL,
  type_id    INTEGER NULL REFERENCES location_types (id) ON DELETE SET NULL,
  parent_id  INTEGER NULL REFERENCES locations (id) ON DELETE SET NULL,
  note       TEXT,
  latitude   REAL,
  longitude  REAL,
  date_start TEXT,  -- same genealogical date text format as events.date
  date_end   TEXT   -- same genealogical date text format as events.date
);
```

### Modify `events`

Migration `002_add_locations.sql`:
```sql
ALTER TABLE events ADD COLUMN location_id INTEGER NULL REFERENCES locations (id) ON DELETE SET NULL;
```

### Schema tags

Add `Schema::LocationTypes` and `Schema::Locations` to `src/database/schema.h`, following the existing `TableTag` pattern.

### Migration registration

Add migration entry (version 2) in `src/database/database.cpp`'s `migrations` array.

---

## Domain Layer

**Files:** `src/domain/location/`

### Entities (`location_entities.h`)

```cpp
struct LocationTypeEntity {
    IntegerPrimaryKey id = -1;
    QString type;
    bool builtin = false;
    static LocationTypeEntity fromSql(const QSqlQuery&);
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
    static LocationEntity fromSql(const QSqlQuery&);
};

// Used for comboboxes — full ancestral path from recursive CTE
struct LocationDisplayEntity {
    IntegerPrimaryKey id = -1;
    QString name;
    QString fullPath;  // e.g. "London > England > United Kingdom"
    static LocationDisplayEntity fromSql(const QSqlQuery&);
};
```

### Repository (`location_repository.h/cpp`)

Inherits `BaseRepository`. Key methods:

**Location type methods** (mirrors `EventRepository` type methods):

| Method | Returns | Notes |
|---|---|---|
| `findAllLocationTypes()` | `QList<LocationTypeEntity>` | |
| `insertLocationType(type)` | `std::optional<IntegerPrimaryKey>` | Notifies `Schema::LocationTypes` |
| `updateLocationType(id, type)` | `bool` | Notifies `Schema::LocationTypes` |
| `deleteLocationType(id)` | `bool` | Notifies `Schema::LocationTypes` |
| `isLocationTypeUsed(id)` | `bool` | Checks `locations.type_id` |

**Location methods:**

| Method | Returns | Notes |
|---|---|---|
| `findAll()` | `QList<LocationEntity>` | All locations, flat |
| `findAllWithPaths()` | `QList<LocationDisplayEntity>` | Recursive CTE, sorted by full path |
| `findById(id)` | `std::optional<LocationEntity>` | |
| `insert(name, typeId, parentId)` | `std::optional<IntegerPrimaryKey>` | Notifies `Schema::Locations` |
| `update(id, name, typeId, parentId, note, lat, lon, dateStart, dateEnd)` | `bool` | Notifies `Schema::Locations` |
| `deleteLocation(id)` | `bool` | Notifies `Schema::Locations` |
| `isUsed(id)` | `bool` | Checks `events.location_id` |
| `findOrCreate(name, typeId, parentId)` | `std::optional<IntegerPrimaryKey>` | Exact match by (name, parentId), creates if not found — for inline creation in event editor |

Recursive CTE for `findAllWithPaths()`:
```sql
WITH RECURSIVE path(id, name, parent_id, full_path) AS (
    SELECT id, name, parent_id, name FROM locations WHERE parent_id IS NULL
    UNION ALL
    SELECT l.id, l.name, l.parent_id, path.full_path || ' > ' || l.name
    FROM locations l JOIN path ON l.parent_id = path.id
)
SELECT id, name, full_path FROM path ORDER BY full_path;
```

### Models

**`LocationTypesListModel`** (`location_types_list_model.h/cpp`) — `ObjectTableModel<LocationTypeEntity>`:
- Columns: `ID`, `TYPE`, `BUILTIN`
- Used in `LocationEditorDialog` type combobox and `LocationTypesManagementWindow`
- `connectToTable<Schema::LocationTypes>(this)` → `reload()`

**`LocationListModel`** (`location_list_model.h/cpp`) — `ObjectTableModel<LocationEntity>`:
- Columns: `ID`, `NAME`, `TYPE_ID`, `PARENT_ID`
- Used with `TreeProxyModel` (`src/utils/tree_proxy_model.h`) for tree views
- `connectToTable<Schema::Locations>(this)` → `reload()`

**`LocationPathsModel`** (`location_paths_model.h/cpp`) — `ObjectTableModel<LocationDisplayEntity>`:
- Columns: `ID`, `FULL_PATH`
- Used in event editor combobox
- `connectToTable<Schema::Locations>(this)` → `reload()`

---

## Location Types Management Window

**Files:** `src/lists/location_types_management_window.h/cpp`

Inherits `SimpleListManagementWindow` (same pattern as `EventTypesManagementWindow`):
- Uses `LocationTypesListModel` / `QSqlTableModel` for the flat list
- Add/Edit/Delete with builtin-item protection
- Delete blocked via `isLocationTypeUsed(id)`

---

## Location Management Window

**Files:** `src/lists/location_management_window.h/cpp`

A `QMainWindow` that follows the same pattern as `SourceTreeWidget` (`src/ui/source/dock/source_list_dock.cpp`):

```
LocationListModel → TreeProxyModel → QSortFilterProxyModel → QTreeView
```

- Search box (QLineEdit, case-insensitive, recursive filtering)
- Toolbar actions: Add root location, Add child (requires selection), Edit selected, Delete selected
- Delete blocked if `isUsed(id)` returns true (shows message box explaining it is in use)
- Add/Edit open `LocationEditorDialog`

---

## Location Editor Dialog

**Files:** `src/editors/location_editor_dialog.h/cpp` + `location_editor_dialog.ui`

Follows the structure of `src/ui/source/editor/source_editor_dialog.h`.

Fields:
- **Name** — QLineEdit
- **Type** — QComboBox backed by `LocationTypesListModel` (TYPE column); nullable, no pre-selection for new locations
- **Parent** — display label showing the current parent's full path + two buttons: "Create new location as parent" (opens a new `LocationEditorDialog`) and "Select existing location" (opens `ChooseExistingLocationWindow`); `std::optional<IntegerPrimaryKey> parentId` stored as a member, same pattern as `SourceEditorDialog::addNewSourceAsParent` / `selectExistingSourceAsParent`
- **Note** — rich text, using `enableRichTextMode()` + "Edit…" button (same as `event_editor_dialog.cpp:109`)
- **Latitude / Longitude** — QDoubleSpinBox (range ±90 / ±180, 6 decimal places)
- **Date start / Date end** — QLineEdit + "Edit with editor" button (same date picker pattern as event editor)

Static factories: `showDialogForNewLocation(parentId, parent)` and `showDialogForExistingLocation(id, parent)`.

**`src/link_existing/choose_existing_location_window.h/cpp`** — inherits `ChooseExistingReferenceWindow` (same pattern as `ChooseExistingSourceWindow`); presents the location tree for selection.

---

## Event Editor Integration

**Modified files:**
- `src/domain/event/event_entities.h` — add `std::optional<IntegerPrimaryKey> locationId` to `EventEntity`
- `src/domain/event/event_repository.h/cpp` — add `locationId` param to `updateEvent()` and `insertFullEvent()`
- `src/editors/event_editor_dialog.h/cpp` — add location combobox
- `src/editors/event_editor_dialog.ui` — add "Location" row to event information group

**Event editor UI additions:**
- QComboBox backed by `LocationPathsModel` (FULL_PATH column, sorted); nullable, no pre-selection when no location assigned
- "New…" button: opens `LocationEditorDialog`, then selects the newly created location

**EventEditorDialog changes:**
- New member `LocationPathsModel* locationsModel`
- On load (existing event): find `event->locationId` in model, `setCurrentIndex` to match
- On `accept()`: read selected ID (nullopt for "none"), pass to `updateEvent`/`insertFullEvent`

---

## Verification

1. **Schema**: open a new database, verify `location_types`, `locations` tables and `events.location_id` column exist; verify builtin types seeded in `location_types`
2. **Migration**: open a pre-existing database (without the migration), verify it is upgraded cleanly
3. **Location types**: open types management window, verify builtin types present, add a custom type, delete it — verify delete blocked if in use
4. **Location CRUD**: open management window, add a root location ("Netherlands", type Country), add a child ("Groningen", type Province), verify tree displays correctly, edit fields, delete unused location
5. **Delete protection**: assign a location to an event, attempt to delete it — verify it is blocked
6. **Event editor**: create a new event, pick a location from the combobox, save — verify `events.location_id` is set. Use "New…" button for inline creation.
7. **Re-open event**: verify the location combobox pre-selects the saved location
8. **Clear location**: change a location to "none", save — verify `events.location_id` is NULL