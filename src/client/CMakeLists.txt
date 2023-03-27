project(client)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
MESSAGE("Begin to build " client)
file(GLOB_RECURSE ALL_SOURCE ../common/*.cpp *.cpp )
file(GLOB_RECURSE ALL_HEAD "*.hpp" "*.h")
MESSAGE(STATUS "ALL_SOURCE IS " ${ALL_SOURCE})
add_executable(client client.cpp ${ALL_SOURCE})
target_include_directories(client
        PRIVATE
        /usr/local/include
        /usr/include
        /usr/lib/gcc/x86_64-linux-gnu/12/include
        )
target_link_libraries(client
        -lpthread
        -lstdc++fs
        )
# aux_source_directory(../server/parse parse_dir_var)
# aux_source_directory(../server/resolve resolve_dir_var)
# aux_source_directory(../server/storage storage_dir_var)
# aux_source_directory(../src src_dir_var)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../../bin)