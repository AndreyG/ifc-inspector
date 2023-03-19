cmake_minimum_required(VERSION 3.26 FATAL_ERROR)
project(ifc-inspector)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dbc3090d2a")

add_subdirectory(lib/ifc)

add_executable(embedfile src/embedfile.cpp)

set(schema_input_file ${CMAKE_SOURCE_DIR}/resources/ifc-reader-interface.h.ifc)
set(schema_output_file ${CMAKE_BINARY_DIR}/schema_raw_data.cpp)

add_custom_command(
  OUTPUT ${schema_output_file}
  COMMAND embedfile schema_raw_data "${CMAKE_BINARY_DIR}" ${schema_input_file}
  DEPENDS ${schema_input_file})

add_executable(ifc-inspector)

target_sources(ifc-inspector
  PRIVATE
    src/main.cpp
    src/commander.cpp
    ${schema_output_file}
  PRIVATE FILE_SET cxx_modules TYPE CXX_MODULES FILES
    src/commander.ixx
)

target_link_libraries(ifc-inspector reflifc ifc-blob-reader)
