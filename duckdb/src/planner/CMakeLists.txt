add_library_unity(
  duckdb_planner
  OBJECT
  bound_result_modifier.cpp
  collation_binding.cpp
  expression_iterator.cpp
  expression.cpp
  logical_operator.cpp
  planner.cpp
  pragma_handler.cpp
  table_filter.cpp)
set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:duckdb_planner>
    PARENT_SCOPE)
