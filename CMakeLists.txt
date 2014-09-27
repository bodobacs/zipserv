cmake_minimum_required(VERSION 2.4.4)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

project(zipserv)

#new way initialization of mimetypes map<> required c++11
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")


find_package( ZLIB REQUIRED )
if( ZLIB_FOUND )
	include_directories( ${ZLIB_INCLUDE_DIRS} )

	#variable set to unzip directory
	set(LIBUNZIP ${CMAKE_SOURCE_DIR}/libs/unzip11)

	#locally builded library minizip/unzip http://www.winimage.com/zLibDll/unzip11.zip depends on zlib
	add_library(unzip ${LIBUNZIP}/unzip.c ${LIBUNZIP}/ioapi.c)
	target_link_libraries(unzip ${ZLIB_LIBRARIES})
endif (ZLIB_FOUND)


#product
add_executable(zipserv mysrv.cpp)

#used static libs
target_link_libraries(zipserv unzip)

#set(VERSION "0.1")

# dependency map
# zipserv < unzip < zlib
