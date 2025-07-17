add_executable(logger_tests test_logger.cpp)

target_link_libraries(logger_tests
    gtest_main
    logger
)

target_include_directories(logger_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/log_lib/include
)

include(GoogleTest)
gtest_discover_tests(logger_tests)
