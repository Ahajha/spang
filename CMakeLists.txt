cmake_minimum_required(VERSION 3.15)
project(spang LANGUAGES CXX)

include(CTest)

if (MSVC)
    add_compile_options(/W4 /WX)
    # Use of std::localtime gives a warning - maybe can revisit in the future
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror)
endif()

add_library(spang STATIC)
target_compile_features(spang PUBLIC cxx_std_20)
target_include_directories(spang PUBLIC include)
target_sources(spang
PUBLIC
    include/spang/dfs.hpp
    include/spang/extend.hpp
    include/spang/graph.hpp
    include/spang/is_min.hpp
    include/spang/logger.hpp
    include/spang/parser.hpp
    include/spang/preprocess.hpp
    include/spang/projection.hpp
    include/spang/utility.hpp
PRIVATE
    source/extend.cpp
    source/is_min.cpp
    source/parser.cpp
    source/preprocess.cpp
    source/projection.cpp
)

add_executable(validate)
target_sources(validate PRIVATE source/exe/validate.cpp)
target_link_libraries(validate PRIVATE spang)

add_subdirectory(test)
