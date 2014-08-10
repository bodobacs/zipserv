cmake_minimum_required(VERSION 2.4.4)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

project(zip-reader)

#saját változó ami  zlib helyére mutat
set(ZLIB ${CMAKE_SOURCE_DIR}/../zlib)

#minizip lefordítása
add_library(unzip ${ZLIB}/contrib/minizip/unzip.c ${ZLIB}/contrib/minizip/ioapi.c)
target_link_libraries(unzip ${ZLIB}/build/libz.a)

#ez csinál egy új target-et
add_executable(zip-reader mysrv.cpp)

#az szükséges statikus könyvtárak megadása
target_link_libraries(zip-reader unzip)

#set(VERSION "0.1")


