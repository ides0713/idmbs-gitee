project(server)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
MESSAGE("Begin to build " server)
add_executable(server server.cpp)
find_package(Boost REQUIRED)
find_library(boost_lib NAMES Boost BOOST Boost_system libboost_thread thread libboost)
message("1312414 ${$boost_lib}")
target_include_directories(server
    PRIVATE        
        /usr/local/include
        ${Boost_INCLUDE_DIR}
)
target_link_directories(server
    PRIVATE
        /usr/local/lib
        /usr/local/boost_1_81_0/lib
)
target_link_libraries(server
    PRIVATE
        -lpthread
        ${Boost_LIBS}
        ${Boost_LIBRARY}
)
aux_source_directory(parse parse_dir_var)
aux_source_directory(storage storage_dir_var)
aux_source_directory(../src src_dir_var)
target_sources(server
    PRIVATE
        ${parse_dir_var}
        ${storage_dir_var}
    PUBLIC
        ${src_dir_var}
)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../../bin)