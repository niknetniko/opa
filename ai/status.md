# Status

Where I left off. Update at the end of every session, even with one line.

Last updated: 2026-10-22

## Focus

Make the data layer pleasant before adding features: one tree model instead of proxy chains, keep view state on
reload, then a generic `QueryModel<T>`.

## Done

- `BaseObjectModel<T, QtModel>` (`src/model/base_object_model.h`): shared items, columns, `data`/`setData`/`flags`/
  `headerData`, `itemAt()`. `ObjectTableModel<T>` is built on it. Tested in `autotests/object_table_model_test.cpp`.

## Next

1. `ObjectTreeModel<T>` on `BaseObjectModel<T, QAbstractItemModel>`, started in `src/model/object_tree_model.h`.
   Tree rule passed in the constructor as `std::variant<GroupBy<T>, ParentLink<T>>`; rebuilds its nodes in
   `rebuild()`. Test with `QAbstractItemModelTester`.
2. Replace `TreeProxyModel` + `VirtualParentsModel` with it (`person_event_tab.cpp`, `source_list_dock.cpp`,
   `location_management_window.cpp`), then delete both proxies and their tests.
3. `KViewStateMaintainer` on list/tree views, so selection, expansion and scroll survive a reload.
4. `QueryModel<T>` (loader + tables it depends on); migrate the models below.
5. Use `itemAt()` in views instead of reading hidden ID columns.
6. Remove the old pattern (below).
7. Import: people, families, events.

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
  group, a removed one leaves an empty group. Goes away with step 2.
- `TreeProxyModel` turns source inserts/removes into resets and ignores `rowsMoved`/`layoutChanged`. Goes away with
  step 2.
- `ObjectTableModel` columns must be defined before `setItems` (asserted); `headerData` is empty for vertical headers.
- Import writes raw SQL on its own connection (repositories are main-thread only); never removes that connection.
- Import errors aren't shown to the user; leftover `QThread::msleep(10)` in `parseGrampsData`.
- Failed queries log a warning and return empty results, so bugs look like "no data".
- `QT_MIN_VERSION` is `6.0.0`; the code needs much newer (dev shell: 6.11).
