enable_testing()

ecm_add_test(
    database.cpp
    LINK_LIBRARIES opa-lib Qt::Test
)

ecm_add_test(
    opa_date.cpp
    LINK_LIBRARIES opa-lib Qt::Test
)

ecm_add_test(
    grouping_model.cpp
    LINK_LIBRARIES opa-lib Qt::Test
)

ecm_add_test(
    custom_sql_relational_model.cpp
    LINK_LIBRARIES opa-lib Qt::Test
)

ecm_add_test(
    builtin_model.cpp
    LINK_LIBRARIES opa-lib Qt::Test
)

ecm_add_test(
    edit_proxy_model.cpp
    LINK_LIBRARIES opa-lib Qt::Test
)

ecm_add_test(
    model_utils_find_source_model_of_type.cpp
    LINK_LIBRARIES opa-lib Qt::Test
)
