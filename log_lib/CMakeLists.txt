option(USE_SHARED "Build shared library" ON)

if(USE_SHARED)
    add_library(logger SHARED
        src/logger.cpp
        include/logger.hpp
    )
else()
    add_library(logger STATIC
        src/logger.cpp
        include/logger.hpp
    )
endif()

target_include_directories(logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
