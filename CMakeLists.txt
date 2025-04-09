cmake_minimum_required(VERSION 3.25)

if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
        "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(GSV VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(QMLExpression REQUIRED)
find_package(QMLModel REQUIRED)

set(GSV_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/GSV)
set(GSV_INTERFACES_DIR ${GSV_SOURCE_DIR}/interfaces)
set(GSV_ADAPTERS_DIR ${GSV_SOURCE_DIR}/gsv-adapters)
set(GSV_CORE_DIR ${GSV_SOURCE_DIR}/gsv-core)
set(GSV_EVALUATOR_DIR ${GSV_SOURCE_DIR}/gsv-evaluator)
set(GSV_LOGGER_DIR ${GSV_SOURCE_DIR}/gsv-logger)
set(GSV_RELATIONS_DIR ${GSV_SOURCE_DIR}/gsv-relations)
set(GSV_UTILS_DIR ${GSV_SOURCE_DIR}/gsv-utils)

# Add subdirectories with component libraries
add_subdirectory(${GSV_UTILS_DIR})
add_subdirectory(${GSV_LOGGER_DIR})
add_subdirectory(${GSV_ADAPTERS_DIR})
add_subdirectory(${GSV_CORE_DIR})
add_subdirectory(${GSV_EVALUATOR_DIR})
add_subdirectory(${GSV_RELATIONS_DIR})

# Define the main interface library
add_library(GSV INTERFACE)
target_include_directories(GSV INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/GSV/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(GSV INTERFACE
    gsv-adapters
    gsv-core
    gsv-evaluator
    gsv-relations
)

# --------- Installation Configuration ---------
# Setup installation directories
include(GNUInstallDirs)

# Install all component libraries and the main interface library
install(TARGETS gsv-adapters gsv-core gsv-evaluator gsv-logger gsv-relations gsv-utils GSV
    EXPORT GSVTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install header files from each component
install(DIRECTORY 
    ${GSV_ADAPTERS_DIR}/include/qml_model_adapter
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GSV/adapters
)
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
    ${GSV_UTILS_DIR}/include/gsv-utils
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY 
    ${GSV_INTERFACES_DIR}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GSV/interfaces
)
install(DIRECTORY 
    ${CMAKE_CURRENT_SOURCE_DIR}/GSV/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GSV
)

# Export targets
install(EXPORT GSVTargets
    FILE GSVTargets.cmake
    NAMESPACE GSV::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GSV
)

# Create and install package config files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/GSVConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Create GSVConfig.cmake.in
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/GSVConfig.cmake.in"
"@PACKAGE_INIT@

include(CMakeFindDependencyMacro)
find_dependency(QMLExpression REQUIRED)
find_dependency(QMLModel REQUIRED)

include(\"\${CMAKE_CURRENT_LIST_DIR}/GSVTargets.cmake\")
check_required_components(GSV)
")

# Configure the config file
configure_package_config_file(
    "${CMAKE_CURRENT_BINARY_DIR}/GSVConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/GSVConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GSV
)

# Install the config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/GSVConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/GSVConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GSV
)