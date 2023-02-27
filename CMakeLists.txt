# cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
# project(idbms LANGUAGES CXX)
# link_directories("/usr/lib/x86_64-linux-gnu")
# include_directories("/usr/local/include")
# include_directories("/home/ubuntu/idbms")
# add_library(parse
#   STATIC
#     server/parser/parse.h
#     server/parser/parse.cpp
#   )
# # add_library(yacc_sql
# #   STATIC
# #     server/parser/yacc_sql.hpp
# #     server/parser/yacc_sql.cpp
# # )
# # add_library(lex_sql
# #   STATIC
# #     server/parser/lex_sql.h
# #     server/parser/lex_sql.cpp
# # )
# add_executable(server server/server.cpp)
# add_executable(client client/client.cpp)
# target_link_libraries(server -lpthread parse)
# target_link_libraries(client -lpthread)
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
PROJECT(idbms)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
MESSAGE("Begin to build " ${PROJECT_NAME})
MESSAGE(STATUS "This is PROJECT_BINARY_DIR dir " ${PROJECT_BINARY_DIR})
MESSAGE(STATUS "This is PROJECT_SOURCE_DIR dir " ${PROJECT_SOURCE_DIR})

ADD_SUBDIRECTORY(client)
ADD_SUBDIRECTORY(server)