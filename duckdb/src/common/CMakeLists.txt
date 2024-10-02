add_subdirectory(exception)

if(${EXIT_TIME_DESTRUCTORS_WARNING})
  set(CMAKE_CXX_FLAGS_DEBUG
      "${CMAKE_CXX_FLAGS_DEBUG} -Wno-exit-time-destructors")
endif()

if(DEBUG_STACKTRACE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDUCKDB_DEBUG_STACKTRACE")
endif()

add_library_unity(
  duckdb_common
  OBJECT
  allocator.cpp
  exception.cpp
  )
set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:duckdb_common>
    PARENT_SCOPE)
