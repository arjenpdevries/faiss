add_library_unity(
  duckdb_main
  OBJECT
  error_manager.cpp
  )
set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:duckdb_main>
    PARENT_SCOPE)
