enable_testing()

ecm_add_tests(
    database.cpp
    opa_date.cpp
    grouping_model.cpp
    custom_sql_relational_model.cpp
    builtin_model.cpp
    edit_proxy_model.cpp
    model_utils_find_source_model_of_type.cpp
    event_editor.cpp
    LINK_LIBRARIES opa-lib Qt::Test
)
