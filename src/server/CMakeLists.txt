project(server)
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
MESSAGE("Begin to build " server)
file(GLOB_RECURSE ALL_SOURCE ../common/*.cpp *.cpp)
file(GLOB_RECURSE ALL_HEAD "*.hpp" "*.h")

add_executable(server server.cpp ${ALL_HEAD} ${ALL_SOURCE})

target_include_directories(server
        PRIVATE
        /usr/local/include
        /usr/include
        /usr/lib/gcc/x86_64-linux-gnu/12/include
        )

target_link_libraries(server
        -lpthread
        -lstdc++fs
        -ljsoncpp
        )
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../../bin)