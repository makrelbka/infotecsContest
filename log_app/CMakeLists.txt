
add_executable(log_app main.cpp)

target_link_libraries(log_app PRIVATE logger)
target_include_directories(log_app PRIVATE ../log_lib/include)
