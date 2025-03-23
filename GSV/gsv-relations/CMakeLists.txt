# gsv-relations, semantic relations definable in GSV
add_library(gsv-relations STATIC)

target_sources(gsv-relations PRIVATE
    ${GSV_RELATIONS_DIR}/src/semantic_relations.cpp
)

# Set BUILD_INTERFACE and INSTALL_INTERFACE for include directories
target_include_directories(gsv-relations 
    PUBLIC
        $<BUILD_INTERFACE:${GSV_RELATIONS_DIR}/include>
        $<INSTALL_INTERFACE:include/GSV/relations>
)

target_link_libraries(gsv-relations PUBLIC
    gsv-evaluator
)