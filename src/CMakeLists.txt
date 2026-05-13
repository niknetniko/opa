add_executable(opa main.cpp)

add_library(
  opa-lib STATIC
  main/main_window.h
  main/main_window.cpp
  database/database.h
  database/database.cpp
  database/schema.h
  domain/person/person_sex.h
  domain/person/person_sex.cpp
  domain/person/person_entities.h
  domain/person/person_repository.h
  domain/person/person_repository.cpp
  domain/person/person_display_model.h
  domain/person/person_display_model.cpp
  domain/person/person_detail_model.h
  domain/person/person_detail_model.cpp
  domain/name/name_entities.h
  domain/name/name_repository.h
  domain/name/name_repository.cpp
  domain/name/name_origins_model.h
  domain/name/name_origins_model.cpp
  domain/name/person_names_model.h
  domain/name/person_names_model.cpp
  domain/event/event_entities.h
  domain/event/event_repository.h
  domain/event/event_repository.cpp
  domain/event/person_events_model.h
  domain/event/person_events_model.cpp
  domain/event/person_birth_events_model.h
  domain/event/person_birth_events_model.cpp
  domain/event/person_death_events_model.h
  domain/event/person_death_events_model.cpp
  domain/event/event_types_model.h
  domain/event/event_types_model.cpp
  domain/event/event_roles_model.h
  domain/event/event_roles_model.cpp
  domain/event/event_list_model.h
  domain/event/event_list_model.cpp
  domain/event/parent_event_roles_list_model.h
  domain/event/parent_event_roles_list_model.cpp
  domain/family/family_entities.h
  domain/family/family_repository.h
  domain/family/family_repository.cpp
  domain/family/ancestor_model.h
  domain/family/ancestor_model.cpp
  domain/family/parents_model.h
  domain/family/parents_model.cpp
  domain/family/family_members_model.h
  domain/family/family_members_model.cpp
  domain/family/family_list_model.h
  domain/family/family_list_model.cpp
  ui/family/family_list_dock.h
  ui/family/family_list_dock.cpp
  domain/media/media_entities.h
  domain/media/media_list_model.h
  domain/media/media_list_model.cpp
  domain/media/media_repository.h
  domain/media/media_repository.cpp
  domain/media/media_service.h
  domain/media/media_service.cpp
  domain/source/source_entities.h
  domain/source/source_repository.h
  domain/source/source_repository.cpp
  domain/source/source_list_model.h
  domain/source/source_list_model.cpp
  domain/source/source_type_entities.h
  domain/source/source_type_translation_entities.h
  domain/source/source_type_translation_repository.h
  domain/source/source_type_translation_repository.cpp
  domain/source/source_types.h
  domain/source/source_types_list_model.h
  domain/source/source_types_list_model.cpp
  domain/location/location_entities.h
  domain/location/location_repository.h
  domain/location/location_repository.cpp
  domain/location/location_types_list_model.h
  domain/location/location_types_list_model.cpp
  domain/location/location_list_model.h
  domain/location/location_list_model.cpp
  domain/location/location_paths_model.h
  domain/location/location_paths_model.cpp
  domain/location/location_types.h
  domain/location/location_types.cpp
  domain/location/location_type_translation_entities.h
  domain/location/location_type_translation_repository.h
  domain/location/location_type_translation_repository.cpp
  domain/event/event_type_translation_entities.h
  domain/event/event_type_translation_repository.h
  domain/event/event_type_translation_repository.cpp
  domain/event/event_role_translation_entities.h
  domain/event/event_role_translation_repository.h
  domain/event/event_role_translation_repository.cpp
  domain/event/event_types.h
  domain/event/event_types.cpp
  person_detail/person_detail_view.h
  person_detail/person_detail_view.cpp
  ui/name/name_editor_dialog.h
  ui/name/name_editor_dialog.cpp
  utils/multi_filter_proxy_model.h
  utils/multi_filter_proxy_model.cpp
  domain/name/names.h
  domain/name/names.cpp
  domain/name/name_origin_translation_entities.h
  domain/name/name_origin_translation_repository.h
  domain/name/name_origin_translation_repository.cpp
  utils/formatted_identifier_delegate.h
  utils/formatted_identifier_delegate.cpp
  person_detail/person_name_tab.h
  person_detail/person_name_tab.cpp
  lists/name_origins_management_window.h
  lists/name_origins_management_window.cpp
  dates/genealogical_date.h
  dates/genealogical_date.cpp
  dates/genealogical_date_proxy_model.h
  dates/genealogical_date_proxy_model.cpp
  dates/genealogical_date_editor_dialog.h
  dates/genealogical_date_editor_dialog.cpp
  person_detail/person_event_tab.h
  person_detail/person_event_tab.cpp
  utils/custom_sql_relational_model.h
  utils/custom_sql_relational_model.cpp
  utils/read_only_proxy_model.h
  utils/read_only_proxy_model.cpp
  utils/builtin_model.cpp
  utils/builtin_model.h
  utils/builtin_text_translating_delegate.h
  utils/builtin_text_translating_delegate.cpp
  lists/event_roles_management_window.h
  lists/event_roles_management_window.cpp
  lists/simple_list_manager.h
  lists/simple_list_manager.cpp
  lists/event_types_management_window.h
  lists/event_types_management_window.cpp
  lists/location_types_management_window.h
  lists/location_types_management_window.cpp
  lists/source_types_management_window.h
  lists/source_types_management_window.cpp
  lists/location_management_window.h
  lists/location_management_window.cpp
  editors/event_editor_dialog.h
  editors/event_editor_dialog.cpp
  editors/location_editor_dialog.h
  editors/location_editor_dialog.cpp
  editors/location_editor_dialog.ui
  editors/type_translations_dialog.h
  editors/type_translations_dialog.cpp
  editors/type_translations_dialog.ui
  welcome/welcome_view.h
  welcome/welcome_view.cpp
  welcome/recent_item_model.h
  welcome/recent_item_model.cpp
  link_existing/choose_existing_reference_window.h
  link_existing/choose_existing_reference_window.cpp
  link_existing/choose_existing_event_window.h
  link_existing/choose_existing_event_window.cpp
  utils/placeholder_widget.h
  utils/placeholder_widget.cpp
  main/person_placeholder_widget.h
  main/person_placeholder_widget.cpp
  editors/note_editor_dialog.h
  editors/note_editor_dialog.cpp
  person_detail/person_family_tab.cpp
  person_detail/person_family_tab.h
  person_detail/person_media_tab.h
  person_detail/person_media_tab.cpp
  utils/model_utils.cpp
  utils/model_utils.h
  tree_view/tree_view_window.cpp
  tree_view/tree_view_window.h
  tree_view/person_tree_graph_model.cpp
  tree_view/person_tree_graph_model.h
  editors/editor_dialog.cpp
  editors/editor_dialog.h
  editors/new_person_editor_dialog.cpp
  editors/new_person_editor_dialog.h
  editors/new_family_editor_dialog.cpp
  editors/new_family_editor_dialog.h
  ui/source/editor/source_editor_dialog.h
  ui/source/editor/source_editor_dialog.cpp
  link_existing/choose_existing_person_window.cpp
  link_existing/choose_existing_person_window.h
  docks/person_list_dock.cpp
  docks/person_list_dock.h
  utils/tree_proxy_model.cpp
  utils/tree_proxy_model.h
  utils/grouping_proxy_model.cpp
  utils/grouping_proxy_model.h
  ui/source/dock/source_list_dock.cpp
  ui/source/dock/source_list_dock.h
  link_existing/choose_existing_source_window.cpp
  link_existing/choose_existing_source_window.h
  link_existing/choose_existing_location_window.cpp
  link_existing/choose_existing_media_window.h
  link_existing/choose_existing_media_window.cpp
  link_existing/choose_existing_location_window.h
  ui/media/media_list_widget.h
  ui/media/media_list_widget.cpp
  ui/media/media_edit_dialog.h
  ui/media/media_edit_dialog.cpp
  ui/media/media_list_dock.h
  ui/media/media_list_dock.cpp
  ui/source/citation_list_widget.h
  ui/source/citation_list_widget.cpp
  utils/pending_list_model.h
  utils/rich_text_plain_delegate.cpp
  utils/rich_text_plain_delegate.h
  utils/type_translation_resolver.h
  utils/type_translation_resolver.cpp
  utils/translating_proxy_model.h
  utils/translating_proxy_model.cpp
  core/query_helper.h
  core/query_helper.cpp
  model/object_table_model.h
  core/data_event_broker.h
  core/data_event_broker.cpp
  core/base_repository.h
  core/query_utils.h
  domain/person/person_detail_model.cpp
  domain/event/event_types_model.cpp
  domain/event/event_roles.h
  domain/event/event_roles.cpp
  domain/source/source.h
  domain/source/source.cpp
  core/keychain_keys.h
  ai/ai_service.h
  ai/ai_service.cpp
  ai/claude_ai_service.h
  ai/claude_ai_service.cpp
  ai/local_ai_service.h
  ai/openai_compatible_service.h
  ai/openai_compatible_service.cpp
  main/ai_settings_widget.h
  main/ai_settings_widget.cpp)

ecm_qt_declare_logging_category(
  opa-lib
  HEADER logging.h
  IDENTIFIER OPA
  CATEGORY_NAME "opa"
  DESCRIPTION "Default log for the application."
  EXPORT Opa)

ecm_qt_export_logging_category(
  IDENTIFIER "OPA_SQL"
  CATEGORY_NAME "opa.sql"
  DESCRIPTION "Opa SQL log"
  EXPORT Opa)

ecm_qt_install_logging_categories(
  EXPORT Opa
  FILE opa.categories
  DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}")

target_link_libraries(opa PRIVATE opa-lib)

kconfig_add_kcfg_files(opa-lib main/opaSettings.kcfgc)
# ki18n_wrap_ui(opa main/settings.ui person_detail/person_detail_view.ui )

target_link_libraries(
  opa-lib
  PUBLIC KF6::CoreAddons
         KF6::I18n
         KF6::XmlGui
         KF6::ConfigWidgets
         KF6::DBusAddons
         KF6::Crash
         KF6::IconThemes
         KF6::ItemModels
         Qt6::Widgets
         Qt6::Sql
         Qt6::Quick
         Qt6::QuickControls2
         Qt6::QuickWidgets
         Qt6::Test
         KF6::KIOCore
         KF6::KIOFileWidgets
         KF6::KIOWidgets
         KF6::TextWidgets
         KF6::WidgetsAddons
         Qt6::Network
         Qt6::Pdf
         Qt6::PdfWidgets
         ${SQLite3_LIBRARIES}
         ${QtNodes_LIBRARIES}
         KDAB::kddockwidgets
         qt6keychain)

qt_add_resources(
  opa-lib "opa-database"
  PREFIX "/"
  BASE "database"
  FILES
    database/init.sql
    database/schema.sql
    database/seed.sql
    database/migrations/001_event_relations_surrogate_key.sql
    database/migrations/002_add_locations.sql
    database/migrations/003_add_type_translations.sql
    database/migrations/004_add_role_origin_translations_and_source_types.sql
    database/migrations/005_add_media.sql
    database/migrations/006_fix_media_note_nullable.sql
    database/migrations/007_add_date_sort.sql
    database/migrations/008_add_families.sql
    database/migrations/009_backfill_family_ids.sql)

install(TARGETS opa ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.example.opa.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.example.opa.appdata.xml
        DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES opaui.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/opa)

# cmake-format: off
# @formatter:off
if(DEFINED CMAKE_CXX_CLANG_TIDY)
# Prevent generated from reporting errors with clang-tidy.
file(WRITE "${CMAKE_BINARY_DIR}/.clang-tidy" "
---
Checks: '-*,llvm-twine-local'
...
")
endif()
# @formatter:on
# cmake-format: on
