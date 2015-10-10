cmake_minimum_required(VERSION 2.4.4)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

project(zserv)
set(VERSION "0.1")

#variable set to minizip directory. This minizip is android compatible. Original name: ndk-unzip-master.
set(LIBUNZIP ${CMAKE_SOURCE_DIR}/../libarchives/minizip)

IF(UNIX)

	#new way initialization of mimetypes map<> required c++11
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

	find_package( ZLIB REQUIRED )
	if( ZLIB_FOUND )
		include_directories( ${ZLIB_INCLUDE_DIRS} )

		#locally builded library minizip/unzip http://www.winimage.com/zLibDll/unzip11.zip depends on zlib
		add_library(unzip ${LIBUNZIP}/unzip.c ${LIBUNZIP}/ioapi.c)
		target_link_libraries(unzip ${ZLIB_LIBRARIES})

	endif (ZLIB_FOUND)

	set(LIBCHM ${CMAKE_SOURCE_DIR}/../libarchives/modchmlib)
	add_library(modchmlib SHARED ${LIBCHM}/chm_lib.c ${LIBCHM}/lzx.c)
#	target_link_libraries(modchmlib ${LIBCHM})

	set(LIBARCHIVES ${CMAKE_SOURCE_DIR}/../libarchives)
	add_library(archives ${LIBARCHIVES}/archives.cpp ${LIBARCHIVES}/archives.h)
	target_link_libraries(archives modchmlib unzip)

	#product
	add_executable(test_archives ${LIBARCHIVES}/test_archives.cpp)
	#used static libs
	target_link_libraries(test_archives archives modchmlib unzip)
	
	#product
	add_executable(zserv ${CMAKE_SOURCE_DIR}/main.cpp ${CMAKE_SOURCE_DIR}/zsrv.cpp ${CMAKE_SOURCE_DIR}/zsrv.h ${CMAKE_SOURCE_DIR}/mimetypes.h ${LIBARCHIVES}/archives.h)
	target_link_libraries(zserv archives)

	#SET(CPACK_GENERATOR "DEB")
	#SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Z") #required

	#include (CPack)


ENDIF(UNIX) #IF(UNIX)
#ELSE() #IF(UNIX)

# ON WINDOWS I used NMAKE, so in vcvars32.bat command line run:
# cd zserv\build32
# cmake -G "NMake Makefiles" ..
# nmake
# For zlib have to set ZLIB_DIR or put it in zserv directory

# zlib build on windows:
# nmake -f win32/Makefile.msc

IF(WIN32)

	IF(MINGW)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
	ELSE(MINGW)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Qstd=c++11")
	ENDIF(MINGW)


	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")

	#zlib PATH HERE
	set(ZLIB_DIR ${CMAKE_SOURCE_DIR}/../../zlib-1.2.8-bin)

	set(CMAKE_PREFIX_PATH ${ZLIB_DIR})

	include_directories( ${ZLIB_DIR}/include )

	#locally builded library minizip/unzip http://www.winimage.com/zLibDll/unzip11.zip depends on zlib
	add_library(unzip ${LIBUNZIP}/unzip.c ${LIBUNZIP}/ioapi.c)
	target_link_libraries(unzip ${ZLIB_DIR}/lib/zdll.lib)

	set(LIBCHM ${CMAKE_SOURCE_DIR}/../libarchives/modchmlib)
	#switch on __declspec(dllexport) in chm_lib.h
	add_definitions(-Dshared_EXPORTS)
	add_library(modchmlib SHARED ${LIBCHM}/chm_lib.c ${LIBCHM}/chm_lib.h ${LIBCHM}/lzx.c)

	##this generates modchmlib's def file to make a dll
	#include (GenerateExportHeader)
	#GENERATE_EXPORT_HEADER( modchmlib )
	#target_include_directories(modchmlib PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

	set(LIBARCHIVES ${CMAKE_SOURCE_DIR}/../libarchives)
	add_library(archives ${LIBARCHIVES}/archives.cpp ${LIBARCHIVES}/archives.h)
	target_link_libraries(archives modchmlib unzip)

	#product
	add_executable(test_archives ${LIBARCHIVES}/test_archives.cpp)
	#used static libs
	target_link_libraries(test_archives archives modchmlib unzip)
	
	#product
	add_executable(zserv_cli zsrv.cpp main.cpp)
	target_link_libraries(zserv_cli archives wsock32 ws2_32)

	#WIN32 -> WinMain not console program!
	#	add_executable(zserv WIN32 zsrv.cpp zserv_dialog.rc winmain.cpp)
	add_executable(zserv zsrv.cpp zserv_dialog.rc winmain.cpp)
	target_link_libraries(zserv archives wsock32 ws2_32)

ENDIF(WIN32) #IF(WIN32)

# dependency map
# zserv < unzip < zlib

# build a CPack driven installer package
#include (InstallRequiredSystemLibraries)
#set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/license.txt")
#set (CPACK_PACKAGE_VERSION_MAJOR "${_VERSION_MAJOR}")
#set (CPACK_PACKAGE_VERSION_MINOR "${Tutorial_VERSION_MINOR}")

