add_library(dfs INTERFACE)

include_directories(../graph)

target_include_directories(dfs INTERFACE include/)
target_link_libraries(dfs PUBLIC graph)
