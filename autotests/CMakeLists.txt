enable_testing()

ecm_add_tests(
  database.cpp
  genealogical_date.cpp
  genealogical_date_proxy_model.cpp
  custom_sql_relational_model.cpp
  builtin_model.cpp
  read_only_proxy_model.cpp
  model_utils_find_source_model_of_type.cpp
  event_editor_dialog.cpp
  family_proxy_model_test.cpp
  ancestor_model_test.cpp
  parent_query_model_test.cpp
  tree_proxy_model.cpp
  grouping_proxy_model.cpp
  LINK_LIBRARIES opa-lib Qt::Test)
