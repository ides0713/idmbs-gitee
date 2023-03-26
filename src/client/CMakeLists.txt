project(client)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
MESSAGE("Begin to build " client)
add_executable(client client.cpp)
target_include_directories(client
        PRIVATE
        /usr/local/include
        ${Boost_INCLUDE_DIR}
        )
target_link_libraries(client
        -lpthread
        ${Boost_LIBRARY}
        )
# aux_source_directory(../server/parse parse_dir_var)
# aux_source_directory(../server/resolve resolve_dir_var)
# aux_source_directory(../server/storage storage_dir_var)
# aux_source_directory(../src src_dir_var)

aux_source_directory(../common common_dir_var)

target_sources(client
        PRIVATE
        PUBLIC
        ${common_dir_var}
        )

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../../bin)