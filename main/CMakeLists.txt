add_executable(validate src/validate.cpp)

target_link_libraries(validate PUBLIC parser logger)
