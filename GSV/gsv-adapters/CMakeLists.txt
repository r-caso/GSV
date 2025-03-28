# gsv-adapters library for IModel compliant classes
find_package(QMLModel REQUIRED)

add_library(gsv-adapters STATIC)

target_sources(gsv-adapters PRIVATE
    ${GSV_ADAPTERS_DIR}/src/qml_model_adapter/qml_model_adapter.cpp
)

# Set BUILD_INTERFACE and INSTALL_INTERFACE for include directories
target_include_directories(gsv-adapters 
    PUBLIC 
        $<BUILD_INTERFACE:${GSV_ADAPTERS_DIR}/include>
        $<BUILD_INTERFACE:${GSV_ADAPTERS_DIR}/include/qml_model_adapter>
        $<BUILD_INTERFACE:${GSV_INTERFACES_DIR}>
        $<INSTALL_INTERFACE:include/GSV/adapters>
        $<INSTALL_INTERFACE:include/GSV/adapters/qml_model_adapter>
)

target_link_libraries(gsv-adapters PUBLIC QMLModel::QMLModel)

# Install adapters.hpp
install(FILES ${GSV_ADAPTERS_DIR}/include/adapters.hpp DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GSV/adapters)