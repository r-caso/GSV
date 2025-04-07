# gsv-evaluator, valuation function for QMLExpressions
find_package(QMLExpression REQUIRED)

add_library(gsv-evaluator STATIC)

target_sources(gsv-evaluator PRIVATE
    ${GSV_EVALUATOR_DIR}/src/evaluator.cpp
)

# Set BUILD_INTERFACE and INSTALL_INTERFACE for include directories
target_include_directories(gsv-evaluator 
    PUBLIC
        $<BUILD_INTERFACE:${GSV_EVALUATOR_DIR}/include>
        $<INSTALL_INTERFACE:include/GSV/evaluator>
)

target_link_libraries(gsv-evaluator PUBLIC 
    gsv-core
    gsv-logger
    QMLExpression::QMLExpression
)