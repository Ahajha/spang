add_library(parser STATIC src/parser.cpp include/parser.hpp)

include_directories(../graph ../logger)

target_include_directories(parser PUBLIC include/)
target_link_libraries(parser graph logger)
