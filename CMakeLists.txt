cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(ifc-inspector)

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
  PRIVATE FILE_SET HEADERS FILES
    src/commander.h
)

target_link_libraries(ifc-inspector reflifc ifc-blob-reader)
