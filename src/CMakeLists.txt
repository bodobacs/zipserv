cmake_minimum_required(VERSION 2.4.4)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

project(zipserv)
set(VERSION "0.1")


IF(UNIX)

	#new way initialization of mimetypes map<> required c++11
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

	find_package( ZLIB REQUIRED )
	if( ZLIB_FOUND )
		include_directories( ${ZLIB_INCLUDE_DIRS} )

		#variable set to minizip directory. This minizip is android compatible. Original name: ndk-unzip-master.
		set(LIBUNZIP ${CMAKE_SOURCE_DIR}/minizip)

		#locally builded library minizip/unzip http://www.winimage.com/zLibDll/unzip11.zip depends on zlib
		add_library(unzip ${LIBUNZIP}/unzip.c ${LIBUNZIP}/ioapi.c)
		target_link_libraries(unzip ${ZLIB_LIBRARIES})

	endif (ZLIB_FOUND)

	#product
	add_executable(zipserv zsrv.cpp main.cpp)

	#used static libs
	target_link_libraries(zipserv unzip)

ELSE() #IF(UNIX)

# ON WINDOWS I used NMAKE so in vcvars32.bat command line run:
# cd zipserv\build32
# cmake -G "NMake Makefiles" ..
# nmake
# For zlib have to set ZLIB_DIR or put it in zipserv directory

# zlib build on windows:
# nmake -f win32/Makefile.msc

IF(WIN32)

	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Qstd=c++11")

	set(ZLIB_DIR ${CMAKE_SOURCE_DIR}/../zlib-1.2.8)

	set(CMAKE_PREFIX_PATH ${ZLIB_DIR})

	include_directories( ${ZLIB_DIR} )

	#variable set to minizip directory. This minizip is android compatible. Original name: ndk-unzip-master.
	set(LIBUNZIP ${CMAKE_SOURCE_DIR}/minizip)

	add_library(unzip ${LIBUNZIP}/unzip.c ${LIBUNZIP}/ioapi.c)
	target_link_libraries(unzip ${ZLIB_DIR}/zlib.lib)

	#product
	add_executable(zipserv zsrv.cpp main.cpp)

	#used static libs
	target_link_libraries(zipserv unzip wsock32 ws2_32)

ENDIF() #IF(WIN32)
ENDIF() #IF(UNIX)


# dependency map
# zipserv < unzip < zlib


# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/license.txt")
#set (CPACK_PACKAGE_VERSION_MAJOR "${_VERSION_MAJOR}")
#set (CPACK_PACKAGE_VERSION_MINOR "${Tutorial_VERSION_MINOR}")

include (CPack)

