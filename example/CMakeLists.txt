file(GLOB_RECURSE CPPS  ./*.cpp )
file(GLOB_RECURSE HPPS  ./*.h )

add_executable(simple_graph_model ${CPPS} ${HPPS})

target_link_libraries(simple_graph_model QtNodes Qt6::Core Qt6::Gui Qt6::Widgets)
target_compile_definitions(simple_graph_model PRIVATE NODE_EDITOR_SHARED=1)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
