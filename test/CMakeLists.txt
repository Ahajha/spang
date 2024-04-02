find_package(Catch2 COMPONENTS Catch2WithMain)

add_executable(unit_tests)
target_sources(unit_tests PRIVATE
    source/test_extend.cpp
    source/test_is_min.cpp
    source/test_parse.cpp
    source/test_preprocess.cpp
)
target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain spang)

add_test(
    NAME unit_tests
    COMMAND unit_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
