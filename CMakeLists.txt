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

add_library(libspang STATIC)
target_compile_features(libspang PUBLIC cxx_std_20)
target_include_directories(libspang PUBLIC include)
target_sources(libspang
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
    source/mine.cpp
    source/parser.cpp
    source/preprocess.cpp
    source/projection.cpp
    source/report.cpp
)

# Use from FetchContent until this is added to Conan
include(FetchContent)
FetchContent_Declare(
    cli151
    GIT_REPOSITORY https://github.com/Ahajha/cli151.git
    GIT_TAG        66c37888b403c2f3434ca7e96248d2f4f74213d6
)
FetchContent_MakeAvailable(cli151)

add_executable(validate)
target_sources(validate PRIVATE source/exe/validate.cpp)
target_link_libraries(validate PRIVATE libspang)

add_executable(spang)
target_sources(spang PRIVATE source/exe/spang.cpp)
target_link_libraries(spang PRIVATE libspang cli151)

add_subdirectory(test)
