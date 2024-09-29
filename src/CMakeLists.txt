add_executable(opa main.cpp resources.qrc
        names/name_editor.ui
)

target_sources(opa PRIVATE
        main/main_window.cpp
        database/database.cpp
        database/schema.h
        main/people_overview_view.cpp
        data/person.cpp
        data/event.cpp
        event/event_table_model.cpp
        event/event_table_view.cpp
        person_detail/person_detail_view.cpp
        utils/gramps_date.cpp
        names/names_overview_view.cpp
        names/name_editor.cpp
        utils/single_row_model.cpp
        data/data_manager.cpp
        data/names.cpp
        utils/formatted_identifier_delegate.cpp
        utils/model_utils.cpp
        person_detail/person_name_tab.cpp
        utils/proxy_enabled_relational_delegate.cpp
)

ecm_qt_declare_logging_category(opa
        HEADER opadebug.h
        IDENTIFIER OPA
        CATEGORY_NAME "opa"
)
kconfig_add_kcfg_files(opa main/opaSettings.kcfgc)
#ki18n_wrap_ui(opa
#        main/settings.ui
#        person_detail/person_detail_view.ui
#        )



target_link_libraries(opa
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

install(TARGETS opa ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.example.opa.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.example.opa.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES opaui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/opa)
