enable_testing()

#add_library(test_utils
#    test_utils.h
#)
#
#target_link_libraries(test_utils PUBLIC
#    opa-lib
#)

ecm_add_tests(
    database.cpp
    genealogical_date.cpp
    genealogical_date_proxy_model.cpp
    grouping_model.cpp
    custom_sql_relational_model.cpp
    builtin_model.cpp
    edit_proxy_model.cpp
    model_utils_find_source_model_of_type.cpp
    event_editor_dialog.cpp
    family_proxy_model_test.cpp
    LINK_LIBRARIES opa-lib Qt::Test
)
