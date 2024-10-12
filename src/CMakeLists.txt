add_executable(opa main.cpp)

add_library(opa-lib STATIC
        main/main_window.cpp
        database/database.cpp
        database/schema.h
        main/people_overview_view.cpp
        data/person.cpp
        data/event.cpp
        person_detail/person_detail_view.cpp
        names/names_overview_view.cpp
        names/name_editor.cpp
        utils/single_row_model.cpp
        data/data_manager.cpp
        data/names.cpp
        utils/formatted_identifier_delegate.cpp
        utils/model_utils.cpp
        person_detail/person_name_tab.cpp
        utils/proxy_enabled_relational_delegate.cpp
        lists/name_origins_management_view.cpp
        utils/opa_date.cpp
        person_detail/person_event_overview_view.cpp
        person_detail/person_event_tab.cpp
        utils/grouped_items_proxy_model.cpp
)

ecm_qt_declare_logging_category(opa-lib
        HEADER opadebug.h
        IDENTIFIER OPA
        CATEGORY_NAME "opa"
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
