# Status

Where I left off. Update at the end of every session, even with one line.

Last updated: 2026-10-23

## Focus

Make the data layer pleasant before adding features: one tree model instead of proxy chains, keep view state on
reload, then a generic `QueryModel<T>`.

## Done

- `BaseObjectModel<T, QtModel>` (`src/model/base_object_model.h`): shared items, columns, `data`/`setData`/`flags`/
  `headerData`, `itemAt()`. `ObjectTableModel<T>` is built on it. Tested in `autotests/object_table_model_test.cpp`.
- `ObjectTreeModel<T>` (`src/model/object_tree_model.h`), written but **not tested yet**. Tree rule is
  `std::variant<Grouper, ParentLink>`. `rebuild()` makes a flat `QList<TreeNode>`: item nodes first (node `i` = item
  `i`), then group nodes; `content` is an item index or a group key. `children`/`rowPosition` are filled in a final
  pass, top-level nodes go in `topLevel`. `internalId()` of a `QModelIndex` = node position. Compiles (checked with a
  throwaway instantiation).
- Tooling: dev shell, `opa` and `run-tidy` on LLVM 23 (`llvmPackages_23`; clang-tidy 22+ skips system headers, much
  faster). `run-tidy` passes extra arguments (`run-tidy -fix -format`). `.clang-tidy`: alias checks off,
  `misc-const-correctness` on, headers now checked. Ran `-fix` over `src/` (const locals, `[[nodiscard]]` on
  repositories, designated initializers, lambda `()` removed, clang-format 23 reformat). Committed in `34e2c5f`.

## Next

1. Finish the tooling change: fix the `[[nodiscard]]` call sites (`new_person_editor_dialog.cpp:81,112` ignore
   `updatePerson`/`deletePerson` results; tests: wrap setup calls in `QVERIFY`), build, run tests.
2. Test `ObjectTreeModel<T>` with `QAbstractItemModelTester`: GroupBy with keys `A, B, A` (expect
   `topLevel = [3, 4]`, `tree[3].children = [0, 2]`, `tree[4].children = [1]`), ungrouped items, ParentLink with a
   parent listed after its child and a missing parent. Leftovers: item that is its own parent → top level; `static_cast<int>` in `rowCount`/`columnCount`; `rowPosition` comment says
   "root"; delete the empty `object_tree_model.cpp` (and its CMake entry).
3. Replace `TreeProxyModel` + `VirtualParentsModel` with it (`person_event_tab.cpp`, `source_list_dock.cpp`,
   `location_management_window.cpp`), then delete both proxies and their tests. Check how `VirtualParentsModel`
   shows group rows (spanned first column? selectable?) so the tab behaves the same.
4. `KViewStateMaintainer` on list/tree views, so selection, expansion and scroll survive a reload.
5. `QueryModel<T>` (loader + tables it depends on); migrate the models below.
6. Use `itemAt()` in views instead of reading hidden ID columns.
7. Remove the old pattern (below).
8. Import: people, families, events.

## Migrations

**Old `QSqlTableModel`/mapper code → repositories**

- [ ] `domain/source/source.*` (`SourcesTableModel`): looks unused, delete
- [ ] `lists/simple_list_manager.*`: looks unused, delete with `utils/custom_sql_relational_model.*`, `utils/builtin_model.*`
- [ ] `utils/model_utils.h` `modelTransaction`: looks unused
- [ ] `editors/editor_dialog.*` (`AbstractEditorDialog`): move `NamesEditorDialog`, `NewPersonEditorDialog` to form-based editing

**`ObjectTableModel` subclasses → `QueryModel<T>`** (all in `src/domain/`)

- [ ] event: `event_list`, `person_events`, `person_birth_events`, `person_death_events`
- [ ] family: `ancestor`, `parents`
- [ ] location: `location_list`, `location_paths`
- [ ] `media_list`, `person_names`, `person_detail`, `person_display`, `source_list`

## Known issues

- `VirtualParentsModel` (grouping) never recalculates its groups: a new role in the person event tab doesn't get a
  group, a removed one leaves an empty group. Goes away with step 3.
- `TreeProxyModel` turns source inserts/removes into resets and ignores `rowsMoved`/`layoutChanged`. Goes away with
  step 3.
- `simple_list_manager.cpp` warns "Could not remove row" when `removeRow` succeeds (missing `!`). File is slated for
  deletion.
- `llvmPackages_23.openmp` fails to build in nixpkgs (standalone build removed upstream); removed from the flake, Opa
  doesn't use OpenMP. Clazy stays on `clangStdenv` (plugin must match its LLVM version).
- `run-tidy -fix` duplicates fixes in headers included by several `.cpp` files (got 14× `[[nodiscard]]`). Fix
  headers through one `.cpp` at a time.
- `ObjectTableModel` columns must be defined before `setItems` (asserted); `headerData` is empty for vertical headers.
- Import writes raw SQL on its own connection (repositories are main-thread only); never removes that connection.
- Import errors aren't shown to the user; leftover `QThread::msleep(10)` in `parseGrampsData`.
- Failed queries log a warning and return empty results, so bugs look like "no data".
- `QT_MIN_VERSION` is `6.0.0`; the code needs much newer (dev shell: 6.11).
