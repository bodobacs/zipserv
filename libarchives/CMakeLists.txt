cmake_minimum_required(VERSION 2.4.4)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

project(zipserv)
set(VERSION "0.1")

set(LIB_ARCHIEVES ${CMAKE_SOURCE_DIRS}/../libarchives)

IF(UNIX)

	#new way initialization of mimetypes map<> required c++11
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

	find_package( ZLIB REQUIRED )
	if( ZLIB_FOUND )
		include_directories( ${ZLIB_INCLUDE_DIRS} )

		#variable set to minizip directory. This minizip is android compatible. Original name: ndk-unzip-master.
		set(LIBUNZIP ${CMAKE_SOURCE_DIR}/../archives/minizip)

		#locally builded library minizip/unzip http://www.winimage.com/zLibDll/unzip11.zip depends on zlib
		#using linux shared zlib!
		add_library(unzip ${LIBUNZIP}/unzip.c ${LIBUNZIP}/ioapi.c)
		target_link_libraries(unzip ${ZLIB_LIBRARIES})

	endif (ZLIB_FOUND)

	#make libchm.so
	add_library(chmlib SHARED ${CMAKE_SOURCE_DIR}/archives/chmlib/chm_lib.c ${CMAKE_SOURCE_DIR}/archives/chmlib/lzx.c)

	#product
	add_executable(test_archive test_archives.cpp archives.cpp)

	#used static libs
	target_link_libraries(test_archive unzip chmlib)

ENDIF()

