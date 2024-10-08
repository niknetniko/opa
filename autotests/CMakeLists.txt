enable_testing()

ecm_add_test(
        database.cpp
        LINK_LIBRARIES opa-lib Qt::Test
        TEST_NAME opadatabasetest
)
