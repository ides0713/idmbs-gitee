project(client)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
MESSAGE("Begin to build " client)
add_executable(client client.cpp)
target_include_directories(client
    PRIVATE        
        /usr/local/include
)
target_link_directories(client
    PRIVATE
        /usr/lib/x86_64-linux-gnu     
)
target_link_libraries(client
    PRIVATE
        -lpthread    
)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../bin)