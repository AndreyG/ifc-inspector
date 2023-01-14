project (reflifc)

file(GLOB_RECURSE headers
  include/reflifc/*.h
)

set(sources
    src/ArrayType.cpp
    src/Declaration.cpp
    src/DyadExpression.cpp
    src/Enumeration.cpp
    src/Enumerator.cpp
    src/Expression.cpp
    src/Field.cpp
    src/Name.cpp
    src/ScopeDeclaration.cpp
    src/StringLiteral.cpp
    src/Struct.cpp
    src/SyntaticType.cpp
    src/TemplateDeclaration.cpp
    src/Type.cpp
    src/Variable.cpp
)

add_library(reflifc STATIC ${sources} ${headers})

target_include_directories(reflifc
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(reflifc PUBLIC ifc-core)