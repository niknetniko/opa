enable_testing()

ecm_add_tests(
  async_test.cpp
  database.cpp
  genealogical_date.cpp
  genealogical_date_proxy_model.cpp
  custom_sql_relational_model.cpp
  read_only_proxy_model.cpp
  model_utils_find_source_model_of_type.cpp
  family_proxy_model_test.cpp
  ancestor_model_test.cpp
  parent_query_model_test.cpp
  tree_proxy_model.cpp
  grouping_proxy_model.cpp
  person_repository_test.cpp
  name_repository_test.cpp
  event_repository_test.cpp
  family_repository_test.cpp
  family_overview_test.cpp
  family_list_model_test.cpp
  source_repository_test.cpp
  location_repository_test.cpp
  pending_list_model_test.cpp
  transaction_batching_test.cpp
  openai_compatible_service_test.cpp
  LINK_LIBRARIES opa-lib Qt::Test)

ecm_add_test(gramps_xml_test.cpp LINK_LIBRARIES opa-lib Qt::Test)
target_compile_definitions(
  gramps_xml_test PRIVATE QTEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

ecm_add_test(gramps_import_media_test.cpp LINK_LIBRARIES opa-lib Qt::Test)
ecm_add_test(gramps_import_repositories_test.cpp LINK_LIBRARIES opa-lib
             Qt::Test)
ecm_add_test(gramps_import_locations_test.cpp LINK_LIBRARIES opa-lib Qt::Test)
