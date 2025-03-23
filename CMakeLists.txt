cmake_minimum_required(VERSION 3.22)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT 
    "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(GSV VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(GSV STATIC)
set(GSV_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/GSV)
set(GSV_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
set(GSV_INTERFACES_DIR ${GSV_SOURCE_DIR}/interfaces)
set(GSV_CORE_DIR ${GSV_SOURCE_DIR}/gsv-core)
set(GSV_EVALUATOR_DIR ${GSV_SOURCE_DIR}/gsv-evaluator)
set(GSV_RELATIONS_DIR ${GSV_SOURCE_DIR}/gsv-relations)

# Add subdirectories with component libraries
add_subdirectory(${GSV_CORE_DIR})
add_subdirectory(${GSV_EVALUATOR_DIR})
add_subdirectory(${GSV_RELATIONS_DIR})

# Define the main interface library
add_library(GSV INTERFACE)
target_link_libraries(GSV INTERFACE
    gsv-core
    gsv-evaluator
    gsv-relations
)

# --------- Installation Configuration ---------
# Setup installation directories
include(GNUInstallDirs)

# Install all component libraries and the main interface library
install(TARGETS gsv-core gsv-evaluator gsv-relations GSV
    EXPORT GSVTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install header files from each component
install(DIRECTORY 
    ${GSV_CORE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GSV/core
)
install(DIRECTORY 
    ${GSV_EVALUATOR_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GSV/evaluator
)
install(DIRECTORY 
    ${GSV_RELATIONS_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GSV/relations
)
install(DIRECTORY 
    ${GSV_INTERFACES_DIR}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GSV/interfaces
)

target_sources(GSV PRIVATE
    GSV/src/evaluator.cpp
    GSV/src/semantic_relations.cpp

    GSV/src/semantics/information_state.cpp
    GSV/src/semantics/possibility.cpp
    GSV/src/semantics/referent_system.cpp

    GSV/src/utility/formatter.cpp
    
    third_party/semantics/model.cpp
    third_party/syntax/expression.cpp
)

target_include_directories(GSV PUBLIC GSV/include GSV/include/interfaces GSV/include/semantics GSV/include/utility third_party/semantics third_party/spdlog/include third_party/syntax)