add_library(preprocess STATIC src/preprocess.cpp include/preprocess.hpp)

target_include_directories(preprocess PUBLIC include/)
target_link_libraries(preprocess PUBLIC parser)
