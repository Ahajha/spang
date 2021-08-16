add_library(dfs INTERFACE)

target_include_directories(dfs INTERFACE include/)
target_link_libraries(dfs PUBLIC graph)
