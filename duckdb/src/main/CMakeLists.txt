add_library_unity(
  duckdb_main
  OBJECT
  attached_database.cpp
  client_config.cpp
  client_context_file_opener.cpp
  client_context_wrapper.cpp
  client_data.cpp
  client_verify.cpp
  chunk_scan_state.cpp
  database_path_and_type.cpp
  error_manager.cpp
  prepared_statement.cpp
  prepared_statement_data.cpp
  # profiling_info.cpp
  valid_checker.cpp)
set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:duckdb_main>
    PARENT_SCOPE)
