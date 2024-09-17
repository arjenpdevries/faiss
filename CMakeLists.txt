cmake_minimum_required(VERSION 3.13)

set(TARGET_NAME faiss)
set(EXTENSION_NAME ${TARGET_NAME}_extension)
set(LOADABLE_EXTENSION_NAME ${TARGET_NAME}_loadable_extension)

project(${TARGET_NAME})

file(
  GLOB
  EXTENSION_SOURCES
  src/*.cpp)

set(EXTENSION_SOURCES ${EXTENSION_SOURCES})

include_directories(src/include)

# create extension targets
build_loadable_extension(${TARGET_NAME} "" ${EXTENSION_SOURCES})
build_static_extension(${TARGET_NAME} ${EXTENSION_SOURCES})

install(
  TARGETS ${EXTENSION_NAME}
  EXPORT "${DUCKDB_EXPORT_SET}"
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}")
