add_executable(opa)

target_sources(opa PRIVATE
        main.cpp
        main/main_window.cpp
        data/database.cpp
        main/people_table_model.cpp
        main/people_table_view.cpp
        data/person.cpp
        data/event.cpp
        event/event_table_model.cpp
        event/event_table_view.cpp
        person_detail/person_detail_view.cpp
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
        KF5::CoreAddons
        KF5::I18n
        KF5::XmlGui
        KF5::ConfigWidgets
        KF5::DBusAddons
        KF5::Crash
        KF5::IconThemes
        Qt5::Widgets
        Qt5::Sql
        )

install(TARGETS opa ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.example.opa.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.example.opa.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES opaui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/opa)
