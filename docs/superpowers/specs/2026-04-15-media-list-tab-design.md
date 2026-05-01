# Media List Tab — Design Spec

**Date:** 2026-04-15

## Context

The app already has a global Source list that opens as a tab in the main view. Media items currently only appear in per-entity detail views (attached to persons, events, sources, etc.). A global media list gives the user a way to browse all media in the file, edit metadata, open files, and delete orphaned or unwanted entries — mirroring the pattern established by the source list.

---

## New Files

| File | Purpose |
|------|---------|
| `src/ui/media/media_list_dock.h/.cpp` | Dock widget containing the global media list UI |
| `src/ui/media/media_edit_dialog.h/.cpp` | Dialog for editing a media item's title and note |

No changes needed to `MediaListModel` or `MediaRepository` — both already exist.

---

## MediaListDock

A `KDDockWidgets::QtWidgets::DockWidget` containing an inner `QWidget` with:

- A `QLineEdit` search box (filters by title/filename)
- A `QTableView` showing all media

**Model stack:**
1. `MediaListModel` (ObjectTableModel<MediaEntity>) — reactive, connected to `Schema::Media` via `DataEventBroker`
2. `QSortFilterProxyModel` — filters on the TITLE column (case-insensitive)

**Interactions:**
- Double-click → opens `MediaEditDialog`
- Right-click context menu:
  - "Edit..." → opens `MediaEditDialog`
  - "Open file" → opens file with `QDesktopServices::openUrl()`
  - "Delete" → confirmation dialog then `MediaRepository::remove(id)`

---

## MediaEditDialog

A `KDE` dialog (inherits `QDialog`, uses `KGuiItem` for buttons) with:

- **Title** — `QLineEdit`, optional (`std::optional<QString>`)
- **Note** — `QPlainTextEdit`, optional
- **Path** — `QLabel`, read-only (for reference)

On accept: calls `MediaRepository::update(entity)`. The `DataEventBroker` notification triggers `MediaListModel` to reload automatically.

---

## Delete Behavior

Triggered from context menu:

1. Show `KMessageBox::questionTwoActions` warning that the record (and all attachment links) will be permanently removed. The file on disk is **not** deleted.
2. On confirm: call `MediaRepository::remove(id)`.
3. All junction table rows (`person_media`, `name_media`, etc.) cascade-delete automatically via SQLite `ON DELETE CASCADE`.
4. `DataEventBroker` notification reloads the model automatically.

---

## Main Window Integration

Follow the exact pattern of `showSourcesList()` in `src/main/main_window.cpp`:

- Add `showMediaListAction_` (declared in `main_window.h`, initialized in constructor)
- Add `showMediaList()` slot:
  - Find existing `MediaListDock` via `findChildren<MediaListDock*>()`
  - If found: bring to front (`setAsCurrentTab()`, or `raise()`/`activateWindow()` if floating)
  - If not found: create new `MediaListDock`, add with `Location_OnRight`
- Add to `syncActions()` — disabled when no file is open
- Register action in `src/opaui.rc` under the same menu as the sources list action

---

## Verification

1. Build with `cmake --build` — no new warnings
2. Open an `.opa` file → "Media list" action becomes enabled in the menu
3. Trigger action → tab opens with all media shown; search box filters rows
4. Double-click a row → `MediaEditDialog` opens pre-populated; saving updates the list
5. Right-click → "Open file" launches the file; "Delete" shows confirmation, removes row on confirm
6. Close and reopen the tab — works correctly (re-creates the dock)
7. Run existing autotests — no regressions
