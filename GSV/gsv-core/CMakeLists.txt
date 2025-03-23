# gsv-core library, semantic primitives
add_library(gsv-core STATIC)

target_sources(gsv-core PRIVATE
    ${GSV_CORE_DIR}/src/information_state.cpp
    ${GSV_CORE_DIR}/src/possibility.cpp
    ${GSV_CORE_DIR}/src/referent_system.cpp
    ${GSV_THIRD_PARTY_DIR}/semantics/model.cpp
)

# Set BUILD_INTERFACE and INSTALL_INTERFACE for include directories
target_include_directories(gsv-core 
    PUBLIC 
        $<BUILD_INTERFACE:${GSV_CORE_DIR}/include>
        $<BUILD_INTERFACE:${GSV_INTERFACES_DIR}>
        $<BUILD_INTERFACE:${GSV_THIRD_PARTY_DIR}/semantics>
        $<INSTALL_INTERFACE:include/GSV/core>
        $<INSTALL_INTERFACE:include/GSV/interfaces>
)