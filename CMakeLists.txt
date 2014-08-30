cmake_minimum_required(VERSION 2.4.4)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

project(zipserv)

#variable set to zlib directory
set(ZLIB ${CMAKE_SOURCE_DIR}/../zlib)

#dependencies: zlib/minizip
add_library(unzip ${ZLIB}/contrib/minizip/unzip.c ${ZLIB}/contrib/minizip/ioapi.c)
target_link_libraries(unzip ${ZLIB}/build/libz.a)

#target
add_executable(zipserv mysrv.cpp)

#used static libs
target_link_libraries(zipserv unzip)

#set(VERSION "0.1")


