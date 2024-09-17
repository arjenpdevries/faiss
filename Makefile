.PHONY: all clean format debug release pull update

all: release

MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
PROJ_DIR := $(dir $(MKFILE_PATH))

# GENERATOR is defined by the ci-tools makefiles

# These flags will make DuckDB build the extension
EXT_NAME=faiss
EXT_NAME_UPPER=FAISS
EXT_CONFIG=${PROJ_DIR}extension_config.cmake
include extension-ci-tools/makefiles/duckdb_extension.Makefile
