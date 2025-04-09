add_library(gsv-utils STATIC)

target_sources(gsv-utils PRIVATE
    ${GSV_UTILS_DIR}/src/error_reporting.cpp
)

# Set BUILD_INTERFACE and INSTALL_INTERFACE for include directories
target_include_directories(gsv-utils 
    PUBLIC
        $<BUILD_INTERFACE:${GSV_UTILS_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
