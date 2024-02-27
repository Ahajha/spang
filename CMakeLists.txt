cmake_minimum_required(VERSION 3.15)
project(spang LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(spang STATIC)
target_include_directories(spang PUBLIC include)
target_sources(spang
PUBLIC
    include/spang/dfs.hpp
    include/spang/graph.hpp
    include/spang/logger.hpp
    include/spang/parser.hpp
    include/spang/preprocess.hpp
PRIVATE
    source/parser.cpp
    source/preprocess.cpp
)

add_executable(validate)
target_sources(validate PRIVATE source/exe/validate.cpp)
target_link_libraries(validate PRIVATE spang)