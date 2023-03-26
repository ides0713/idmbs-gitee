project(server)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
MESSAGE("Begin to build " server)
add_executable(server server.cpp execute/execute_main.h)

target_include_directories(server
        PRIVATE
        /usr/local/include
        )
target_link_libraries(server
        PRIVATE
        -lpthread
        )
aux_source_directory(parse parse_dir_var)
aux_source_directory(resolve resolve_dir_var)
aux_source_directory(execute execute_dir_var)
aux_source_directory(storage storage_dir_var)
aux_source_directory(common server_common_dir_var)
aux_source_directory(../common common_dir_var)

target_sources(server
        PRIVATE
        ${parse_dir_var}
        ${storage_dir_var}
        ${resolve_dir_var}
        ${execute_dir_var}
        ${server_common_dir_var}
        PUBLIC
        ${common_dir_var}
        )
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../../bin)