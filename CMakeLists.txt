cmake_minimum_required(VERSION 3.22)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT 
    "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(GSV VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(GSV STATIC)

target_sources(GSV PRIVATE
    GSV/src/evaluator.cpp
    GSV/src/semantic_relations.cpp

    GSV/src/semantics/information_state.cpp
    GSV/src/semantics/possibility.cpp
    GSV/src/semantics/referent_system.cpp
    
    third_party/semantics/model.cpp
    third_party/syntax/expression.cpp
    third_party/syntax/variable.cpp
)

target_include_directories(GSV PUBLIC GSV/include GSV/include/interfaces GSV/include/semantics third_party/semantics third_party/syntax)