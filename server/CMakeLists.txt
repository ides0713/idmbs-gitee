project(server)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
MESSAGE("Begin to build " server)
add_executable(server server.cpp)
find_package(Boost COMPONENTS regex thread REQUIRED)
SET (BOOST_ROOT "/usr/local/boost_1_81_0")
SET (BOOST_INCLUDEDIR "/usr/local/boost_1_81_0/include")
SET (BOOST_LIBRARYDIR "/usr/local/boost_1_81_0/lib")
MESSAGE(STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}")
MESSAGE(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}")
target_include_directories(server
    PRIVATE        
        /usr/local/include
        ${Boost_INCLUDE_DIR}
)
target_link_libraries(server
    PRIVATE
        -lpthread
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