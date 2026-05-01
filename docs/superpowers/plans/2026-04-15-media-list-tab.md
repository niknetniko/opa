# Media List Tab Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a global media list dock tab to the main window, following the same pattern as the source list, with search, double-click to edit, and a right-click context menu for editing, opening, and deleting media.

**Architecture:** A `MediaListDock` (new) wraps an inner widget with a `QTableView` backed by the existing `MediaListModel` + `QSortFilterProxyModel`. A new `MediaEditDialog` handles editing title and note. The main window gains a `showMediaList()` slot and `showMediaListAction_` following the exact pattern of `showSourcesList()`.

**Tech Stack:** C++20, Qt 6, KDE Frameworks 6 (KLocalizedString, KMessageBox), KDDockWidgets, SQLite via existing `MediaRepository`.

---

## File Map

| Action | File |
|--------|------|
| Create | `src/ui/media/media_edit_dialog.h` |
| Create | `src/ui/media/media_edit_dialog.cpp` |
| Create | `src/ui/media/media_list_dock.h` |
| Create | `src/ui/media/media_list_dock.cpp` |
| Modify | `src/main/main_window.h` |
| Modify | `src/main/main_window.cpp` |
| Modify | `src/opaui.rc` |
| Modify | `src/CMakeLists.txt` |

---

### Task 1: Create `MediaEditDialog`

**Files:**
- Create: `src/ui/media/media_edit_dialog.h`
- Create: `src/ui/media/media_edit_dialog.cpp`

- [ ] **Step 1: Write `media_edit_dialog.h`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"
#include "domain/media/media_entities.h"

#include <QDialog>
#include <optional>

class QLineEdit;
class QPlainTextEdit;

class MediaEditDialog : public QDialog {
    Q_OBJECT

public:
    explicit MediaEditDialog(const MediaEntity& entity, QWidget* parent = nullptr);

public Q_SLOTS:
    void accept() override;

private:
    IntegerPrimaryKey mediaId;
    QLineEdit* titleEdit;
    QPlainTextEdit* noteEdit;
};
```

- [ ] **Step 2: Write `media_edit_dialog.cpp`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "media_edit_dialog.h"

#include "domain/media/media_repository.h"

#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

MediaEditDialog::MediaEditDialog(const MediaEntity& entity, QWidget* parent) :
    QDialog(parent),
    mediaId(entity.id) {
    setWindowTitle(i18n("Edit Media"));

    auto* formLayout = new QFormLayout;

    titleEdit = new QLineEdit(this);
    titleEdit->setPlaceholderText(i18n("Optional title"));
    if (entity.title.has_value()) {
        titleEdit->setText(*entity.title);
    }
    formLayout->addRow(i18n("Title:"), titleEdit);

    noteEdit = new QPlainTextEdit(this);
    if (entity.note.has_value()) {
        noteEdit->setPlainText(*entity.note);
    }
    formLayout->addRow(i18n("Note:"), noteEdit);

    auto* pathLabel = new QLabel(entity.path, this);
    pathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    formLayout->addRow(i18n("File:"), pathLabel);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &MediaEditDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &MediaEditDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(formLayout);
    layout->addWidget(buttons);
}

void MediaEditDialog::accept() {
    const auto titleText = titleEdit->text().trimmed();
    const std::optional<QString> title = titleText.isEmpty() ? std::nullopt : std::make_optional(titleText);

    const auto noteText = noteEdit->toPlainText().trimmed();
    const std::optional<QString> note = noteText.isEmpty() ? std::nullopt : std::make_optional(noteText);

    MediaRepository repo;
    repo.update(mediaId, title, note);

    QDialog::accept();
}
```

- [ ] **Step 3: Add files to `src/CMakeLists.txt`**

Find the block that lists `ui/media/media_list_widget.h` and `ui/media/media_list_widget.cpp` and add the two new files after them:

```cmake
  ui/media/media_list_widget.h
  ui/media/media_list_widget.cpp
  ui/media/media_edit_dialog.h
  ui/media/media_edit_dialog.cpp
```

- [ ] **Step 4: Build and verify it compiles**

```bash
cd /home/niko/Ontwikkeling/opa && cmake --build build 2>&1 | tail -20
```

Expected: No errors related to `media_edit_dialog`.

- [ ] **Step 5: Commit**

```bash
git add src/ui/media/media_edit_dialog.h src/ui/media/media_edit_dialog.cpp src/CMakeLists.txt
git commit -m "feat: add MediaEditDialog for editing media title and note"
```

---

### Task 2: Create `MediaListDock`

**Files:**
- Create: `src/ui/media/media_list_dock.h`
- Create: `src/ui/media/media_list_dock.cpp`

- [ ] **Step 1: Write `media_list_dock.h`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <kddockwidgets/qtwidgets/views/DockWidget.h>

#include <QTableView>
#include <QWidget>

class MediaTableWidget : public QWidget {
    Q_OBJECT

public:
    explicit MediaTableWidget(QWidget* parent = nullptr);

private Q_SLOTS:
    void onContextMenuRequested(const QPoint& pos);
    void onDoubleClicked(const QModelIndex& index);

private:
    QTableView* tableView;
};

class MediaListDock : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit MediaListDock();
};
```

- [ ] **Step 2: Write `media_list_dock.cpp`**

```cpp
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "media_list_dock.h"

#include "domain/media/media_entities.h"
#include "domain/media/media_list_model.h"
#include "domain/media/media_repository.h"
#include "domain/media/media_service.h"
#include "ui/media/media_edit_dialog.h"

#include <KLocalizedString>
#include <QDesktopServices>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QUrl>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

MediaTableWidget::MediaTableWidget(QWidget* parent) : QWidget(parent) {
    auto* model = new MediaListModel(this);

    auto* filtered = new QSortFilterProxyModel(this);
    filtered->setSourceModel(model);
    filtered->setFilterKeyColumn(MediaListModel::TITLE);
    filtered->setFilterCaseSensitivity(Qt::CaseInsensitive);

    auto* searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText(i18n("Search.."));
    searchBox->setClearButtonEnabled(true);
    connect(searchBox, &QLineEdit::textEdited, filtered, &QSortFilterProxyModel::setFilterFixedString);

    tableView = new QTableView(this);
    tableView->setModel(filtered);
    tableView->setSelectionBehavior(QTableView::SelectRows);
    tableView->setSelectionMode(QTableView::SingleSelection);
    tableView->setSortingEnabled(true);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->horizontalHeader()->setSectionResizeMode(MediaListModel::TITLE, QHeaderView::Stretch);
    tableView->hideColumn(MediaListModel::ID);
    tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    tableView->setUniformRowHeights(true);

    connect(tableView, &QTableView::customContextMenuRequested, this, &MediaTableWidget::onContextMenuRequested);
    connect(tableView, &QTableView::doubleClicked, this, &MediaTableWidget::onDoubleClicked);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(searchBox);
    layout->addWidget(tableView);
}

void MediaTableWidget::onDoubleClicked(const QModelIndex& index) {
    if (!index.isValid()) {
        return;
    }
    const auto mediaId =
        index.siblingAtColumn(MediaListModel::ID).data(Qt::EditRole).value<IntegerPrimaryKey>();
    MediaRepository repo;
    auto entity = repo.findById(mediaId);
    if (!entity.has_value()) {
        return;
    }
    auto* dialog = new MediaEditDialog(*entity, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void MediaTableWidget::onContextMenuRequested(const QPoint& pos) {
    const auto index = tableView->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    const auto mediaId =
        index.siblingAtColumn(MediaListModel::ID).data(Qt::EditRole).value<IntegerPrimaryKey>();

    QMenu menu(tableView);

    auto* editAction = new QAction(i18n("&Edit..."), &menu);
    editAction->setIcon(QIcon::fromTheme(u"document-edit"_s));
    connect(editAction, &QAction::triggered, this, [this, mediaId]() {
        MediaRepository repo;
        auto entity = repo.findById(mediaId);
        if (!entity.has_value()) {
            return;
        }
        auto* dialog = new MediaEditDialog(*entity, this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->exec();
    });
    menu.addAction(editAction);

    auto* openAction = new QAction(i18n("&Open File"), &menu);
    openAction->setIcon(QIcon::fromTheme(u"document-open"_s));
    connect(openAction, &QAction::triggered, this, [mediaId]() {
        MediaRepository repo;
        auto entity = repo.findById(mediaId);
        if (!entity.has_value()) {
            return;
        }
        const auto absolutePath = MediaService::instance().resolveAbsolutePath(entity->path);
        QDesktopServices::openUrl(QUrl::fromLocalFile(absolutePath));
    });
    menu.addAction(openAction);

    menu.addSeparator();

    auto* deleteAction = new QAction(i18n("&Delete"), &menu);
    deleteAction->setIcon(QIcon::fromTheme(u"edit-delete"_s));
    connect(deleteAction, &QAction::triggered, this, [this, mediaId]() {
        const auto result = QMessageBox::warning(
            this,
            i18n("Delete Media"),
            i18n("Are you sure you want to delete this media record? The file on disk will not be removed."),
            QMessageBox::Yes | QMessageBox::No
        );
        if (result == QMessageBox::Yes) {
            MediaRepository repo;
            Q_UNUSED(repo.remove(mediaId));
        }
    });
    menu.addAction(deleteAction);

    menu.exec(tableView->viewport()->mapToGlobal(pos));
}

MediaListDock::MediaListDock() :
    DockWidget(QStringLiteral("Media"), KDDockWidgets::DockWidgetOption_DeleteOnClose) {
    setWidget(new MediaTableWidget(this));
}
```

- [ ] **Step 3: Add files to `src/CMakeLists.txt`**

After `ui/media/media_edit_dialog.cpp` (added in Task 1), add:

```cmake
  ui/media/media_list_dock.h
  ui/media/media_list_dock.cpp
```

- [ ] **Step 4: Build and verify it compiles**

```bash
cd /home/niko/Ontwikkeling/opa && cmake --build build 2>&1 | tail -20
```

Expected: No errors related to `media_list_dock`.

- [ ] **Step 5: Commit**

```bash
git add src/ui/media/media_list_dock.h src/ui/media/media_list_dock.cpp src/CMakeLists.txt
git commit -m "feat: add MediaListDock with search, edit, open, and delete"
```

---

### Task 3: Integrate into `MainWindow`

**Files:**
- Modify: `src/main/main_window.h`
- Modify: `src/main/main_window.cpp`

- [ ] **Step 1: Add forward declaration and slot to `main_window.h`**

After the line `void showSourcesList();` (around line 110), add:

```cpp
    void showMediaList();
```

After the line `QAction* showSourcesListAction_ = nullptr;` (around line 135), add:

```cpp
    QAction* showMediaListAction_ = nullptr;
```

- [ ] **Step 2: Add action init and `showMediaList()` to `main_window.cpp`**

In the constructor, after the block that sets up `showSourcesListAction_` (around line 116–119), add:

```cpp
    showMediaListAction_ = new QAction(this);
    showMediaListAction_->setText(i18n("Show media list"));
    showMediaListAction_->setIcon(QIcon::fromTheme(QStringLiteral("folder-images")));
    connect(showMediaListAction_, &QAction::triggered, this, &MainWindow::showMediaList);
```

In the `actionCollection->addAction(...)` block, after the line registering `show_sources_list` (around line 132), add:

```cpp
    actionCollection->addAction(QStringLiteral("show_media_list"), showMediaListAction_);
```

In `syncActions()`, add `showMediaListAction_` to the `fileActions` list (around line 205):

```cpp
    const QList fileActions = {
        manageEventRoles_,
        manageEventTypes_,
        manageNameOrigins_,
        manageLocations_,
        manageLocationTypes_,
        manageSourceTypes_,
        addNewPersonAction_,
        addNewSourceAction_,
        showPeopleListAction_,
        showSourcesListAction_,
        showMediaListAction_,
    };
```

Add the `showMediaList()` implementation after `showSourcesList()` (around line 505):

```cpp
void MainWindow::showMediaList() {
    auto dockWidgets = findChildren<MediaListDock*>();
    if (!dockWidgets.empty()) {
        auto* dock = dockWidgets.first();
        if (dock->isFloating()) {
            dock->raise();
            dock->activateWindow();
        }
        dock->setAsCurrentTab();
        return;
    }

    auto* container = getMainDockHost();
    auto* mediaListDock = new MediaListDock;
    container->addDockWidget(mediaListDock, KDDockWidgets::Location_OnRight);

    syncActions();
}
```

Add the include for `MediaListDock` near the other dock includes at the top of `main_window.cpp`:

```cpp
#include "ui/media/media_list_dock.h"
```

- [ ] **Step 3: Build and verify it compiles**

```bash
cd /home/niko/Ontwikkeling/opa && cmake --build build 2>&1 | tail -20
```

Expected: Clean build.

- [ ] **Step 4: Commit**

```bash
git add src/main/main_window.h src/main/main_window.cpp
git commit -m "feat: add showMediaList() slot and showMediaListAction to MainWindow"
```

---

### Task 4: Register action in `opaui.rc` and verify end-to-end

**Files:**
- Modify: `src/opaui.rc`

- [ ] **Step 1: Add `show_media_list` to the view menu in `src/opaui.rc`**

Change the `view` menu from:

```xml
    <Menu name="view">
      <text>Manage</text>
      <Action name="show_people_list"/>
      <Action name="show_sources_list"/>
    </Menu>
```

To:

```xml
    <Menu name="view">
      <text>Manage</text>
      <Action name="show_people_list"/>
      <Action name="show_sources_list"/>
      <Action name="show_media_list"/>
    </Menu>
```

Also bump the `version` attribute on the `<gui>` element by 1 (e.g. `version="4"` → `version="5"`). KDE requires this whenever the UI structure changes.

- [ ] **Step 2: Build**

```bash
cd /home/niko/Ontwikkeling/opa && cmake --build build 2>&1 | tail -20
```

Expected: Clean build.

- [ ] **Step 3: Manual end-to-end verification**

1. Run the application and open an `.opa` file.
2. Verify the "Show media list" action appears in the View menu and is enabled.
3. Click it — a "Media" tab opens showing all media rows, with ID column hidden.
4. Type in the search box — rows filter by title/filename.
5. Double-click a row — `MediaEditDialog` opens pre-populated with title, note, and path.
6. Edit title/note and click OK — the list updates automatically.
7. Right-click a row — menu shows "Edit...", "Open File", "Delete".
8. Click "Open File" — system opens the file.
9. Click "Delete" — confirmation dialog appears; confirm — row disappears.
10. Close the Media tab and reopen via the menu — works correctly.
11. Close the file — "Show media list" action becomes disabled.

- [ ] **Step 4: Commit**

```bash
git add src/opaui.rc
git commit -m "feat: register show_media_list action in view menu"
```
