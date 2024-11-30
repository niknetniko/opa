add_executable(opa main.cpp)

add_library(opa-lib STATIC
    main/main_window.h
    main/main_window.cpp
    database/database.h
    database/database.cpp
    database/schema.h
    main_person_tab/person_list.h
    main_person_tab/person_list.cpp
    data/person.h
    data/person.cpp
    data/event.h
    data/event.cpp
    person_detail/person_detail_view.h
    person_detail/person_detail_view.cpp
    names/name_editor.h
    names/name_editor.cpp
    utils/single_row_model.h
    utils/single_row_model.cpp
    data/data_manager.h
    data/data_manager.cpp
    data/names.h
    data/names.cpp
    utils/formatted_identifier_delegate.h
    utils/formatted_identifier_delegate.cpp
    utils/model_utils_find_source_model_of_type.h
    person_detail/person_name_tab.h
    person_detail/person_name_tab.cpp
    utils/proxy_enabled_relational_delegate.h
    utils/proxy_enabled_relational_delegate.cpp
    lists/name_origins_management_window.h
    lists/name_origins_management_window.cpp
    dates/genealogical_date.h
    dates/genealogical_date.cpp
    dates/genealogical_date_proxy_model.h
    dates/genealogical_date_proxy_model.cpp
    dates/genealogical_date_edit_window.h
    dates/genealogical_date_edit_window.cpp
    person_detail/person_event_overview_view.h
    person_detail/person_event_overview_view.cpp
    person_detail/person_event_tab.h
    person_detail/person_event_tab.cpp
    utils/grouped_items_proxy_model.h
    utils/grouped_items_proxy_model.cpp
    utils/custom_sql_relational_model.h
    utils/custom_sql_relational_model.cpp
    utils/edit_proxy_model.h
    utils/edit_proxy_model.cpp
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
    events/event_editor.h
    events/event_editor.cpp
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
    utils/proxy_delegate.h
    utils/proxy_delegate.cpp
    notes/note_editor_window.h
    notes/note_editor_window.cpp
    data/family.h
    data/family.cpp
    person_detail/person_family_tab.cpp
    person_detail/person_family_tab.h
    utils/model_utils.cpp
    utils/model_utils.h
)

ecm_qt_declare_logging_category(opa-lib
    HEADER logging.h
    IDENTIFIER OPA
    CATEGORY_NAME "opa"
    DESCRIPTION "Default log for the application."
    EXPORT Opa
)

ecm_qt_export_logging_category(
    IDENTIFIER "OPA_SQL"
    CATEGORY_NAME "opa.sql"
    DESCRIPTION "Opa SQL log"
    EXPORT Opa
)

ecm_qt_install_logging_categories(
    EXPORT Opa
    FILE opa.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

target_link_libraries(opa PRIVATE
    opa-lib
)

kconfig_add_kcfg_files(opa-lib main/opaSettings.kcfgc)
#ki18n_wrap_ui(opa
#        main/settings.ui
#        person_detail/person_detail_view.ui
#        )

target_link_libraries(opa-lib PUBLIC
    KF6::CoreAddons
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
    ${SQLite3_LIBRARIES}
)

qt_add_resources(opa-lib "opa-database"
    PREFIX "/"
    BASE "database"
    FILES database/init.sql database/schema.sql
)

install(TARGETS opa ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.example.opa.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.example.opa.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES opaui.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/opa)

# @formatter:off
if (DEFINED CMAKE_CXX_CLANG_TIDY)
# Prevent generated from reporting errors with clang-tidy.
file(WRITE "${CMAKE_BINARY_DIR}/.clang-tidy" "
---
Checks: '-*,llvm-twine-local'
...
")
endif ()
# @formatter:on
