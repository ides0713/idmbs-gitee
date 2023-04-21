project(client)
set(CMAKE_CXX_STANDARD 20)
MESSAGE("Begin to build " client)
file(GLOB_RECURSE ALL_SOURCE ../common/*.cpp *.cpp)
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
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../../bin)