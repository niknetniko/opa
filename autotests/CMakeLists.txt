enable_testing()

ecm_add_test(
        database.cpp
        LINK_LIBRARIES opa-lib Qt::Test
        TEST_NAME database_test
)

ecm_add_test(
        opa_date.cpp
        LINK_LIBRARIES opa-lib Qt::Test
        TEST_NAME opa_date_test
)

ecm_add_test(
        grouping_model.cpp
        LINK_LIBRARIES opa-lib Qt::Test
        TEST_NAME grouped_items_proxy_model_test
)
