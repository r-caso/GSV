# gsv-logger module — internal logging utilities
add_library(gsv-logger INTERFACE)

# Setup interface include directories
target_include_directories(gsv-logger 
    INTERFACE
        $<BUILD_INTERFACE:${GSV_LOGGER_DIR}/include>
)