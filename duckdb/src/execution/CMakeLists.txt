add_subdirectory(expression_executor)
add_subdirectory(nested_loop_join)
add_subdirectory(index)

add_library_unity(
  duckdb_execution
  OBJECT
  adaptive_filter.cpp
  column_binding_resolver.cpp
  expression_executor.cpp
  expression_executor_state.cpp
  # physical_plan_generator.cpp
  reservoir_sample.cpp
  window_executor.cpp
  window_segment_tree.cpp)
set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:duckdb_execution>
    PARENT_SCOPE)
