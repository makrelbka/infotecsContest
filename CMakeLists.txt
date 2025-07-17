cmake_minimum_required(VERSION 3.10)
project(LogProject)

set(CMAKE_CXX_STANDARD 17)


option(USE_SHARED "Build shared logger library" ON)

enable_testing()

add_subdirectory(log_lib)
add_subdirectory(log_app)
add_subdirectory(stat_app)
add_subdirectory(external/googletest)
add_subdirectory(log_tests)



