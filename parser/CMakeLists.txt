add_library(parser STATIC src/parser.cpp include/parser.hpp)

target_include_directories(parser PUBLIC include/)
target_link_libraries(parser graph logger)
