cmake_minimum_required(VERSION 2.4.4)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

project(zserv)

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
	add_executable(zserv_cli ${CMAKE_SOURCE_DIR}/main.cpp ${CMAKE_SOURCE_DIR}/zsrv.cpp ${CMAKE_SOURCE_DIR}/zsrv.h ${CMAKE_SOURCE_DIR}/mimetypes.h ${LIBARCHIVES}/archives.h)
	target_link_libraries(zserv_cli archives)


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
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
#		set(CMAKE_EXE_LINKER_FLAGS "-static")
	ELSE(MINGW)
		#set(CMAKE_GENERATOR "NMake Makefiles")
		#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Qstd=c++11")
	ENDIF(MINGW)


	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")

	#zlib PATH HERE build it or download it
	set(ZLIB_DIR ${CMAKE_SOURCE_DIR}/../external/zlib-bin)

	include_directories( ${ZLIB_DIR}/include )

	#locally builded library minizip/unzip http://www.winimage.com/zLibDll/unzip11.zip depends on zlib
	add_library(unzip ${LIBUNZIP}/unzip.c ${LIBUNZIP}/ioapi.c)
	target_link_libraries(unzip ${ZLIB_DIR}/lib/zdll.lib)

	#modified chm_lib in dll (LPGL)
	set(LIBCHM ${CMAKE_SOURCE_DIR}/../libarchives/modchmlib)
	#switch on __declspec(dllexport) in chm_lib.h
	add_definitions(-Dshared_EXPORTS)
	add_library(modchmlib SHARED ${LIBCHM}/chm_lib.c ${LIBCHM}/chm_lib.h ${LIBCHM}/lzx.c)

	##this generates modchmlib's def file to make a dll
	#include (GenerateExportHeader)
	#GENERATE_EXPORT_HEADER( modchmlib )
	#target_include_directories(modchmlib PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

	#static common interface for zlib and chm_lib
	set(LIBARCHIVES ${CMAKE_SOURCE_DIR}/../libarchives)
	add_library(archives ${LIBARCHIVES}/archives.cpp ${LIBARCHIVES}/archives.h)
	target_link_libraries(archives modchmlib unzip)

	#product testing libarchives, listing archives content
	add_executable(test_archives ${LIBARCHIVES}/test_archives.cpp)
	#used static libs
	target_link_libraries(test_archives archives)
	
	#product
	add_executable(zserv_cli zsrv.cpp main.cpp)
	target_link_libraries(zserv_cli archives wsock32 ws2_32)

	#WIN32 -> WinMain not console program!
	add_executable(zserv WIN32 zsrv.cpp zserv_dialog.rc winmain.cpp)

	target_link_libraries(zserv archives wsock32 ws2_32)

	#add_executable(zserv-debug zsrv.cpp zserv_dialog.rc winmain.cpp)
	#target_link_libraries(zserv-debug archives wsock32 ws2_32)

	#on windows just put everything in installdir
	install(TARGETS zserv zserv_cli modchmlib
		RUNTIME
		DESTINATION "./")

	install(FILES ${ZLIB_DIR}/zlib1.dll
		DESTINATION "./")

	add_custom_target(create_zip COMMAND
		${CMAKE_COMMAND} -E tar "cfv" "archive.zip" --format=zip
	       "${CMAKE_CURRENT_SOURCE_DIR}/testfile.txt"
	       "${CMAKE_CURRENT_SOURCE_DIR}/testdir")


ENDIF(WIN32) #IF(WIN32)

# dependency map
# zserv < unzip < zlib

INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_NAME "zserv-reader")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Zserv ebook reader, works like a webserver.")
SET(CPACK_PACKAGE_VENDOR "Bigplayer")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}")

IF(WIN32 AND NOT UNIX)
# There is a bug in NSI that does not handle full unix paths properly. Make
# sure there is at least one set of four (4) backlasshes.
	#SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/..\\\\InstallIcon.bmp")
	SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\zserv.exe")
	SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_NAME}")
	SET(CPACK_NSIS_HELP_LINK "https:\\\\\\\\sites.google.com/site/zservreader/")
	SET(CPACK_NSIS_URL_INFO_ABOUT "https:\\\\\\\\sites.google.com/site/zservreader/")
	SET(CPACK_NSIS_CONTACT "bigplayer.dev@gmail.com")
	SET(CPACK_NSIS_MODIFY_PATH ON)

	#set(CPACK_GENERATOR "ZIP;NSIS")
	#set(CPACK_PACKAGE_EXECUTABLES zserv "zserv-reader")
	SET(CPACK_PACKAGE_EXECUTABLES "zserv" "zserv-reader")

ELSE(WIN32 AND NOT UNIX)
	SET(CPACK_STRIP_FILES "bin/zserv")
	SET(CPACK_SOURCE_STRIP_FILES "")
	set(CPACK_PACKAGE_NAME zserv_cli)
	set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Small server can be used to read zip and chm files in browser.")
	set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../readme.txt")
	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../copyright.txt")
	set(MY_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
	set(CPACK_MONOLITHIC_INSTALL 1)
	set(CPACK_GENERATOR "TGZ;DEB")

	install( 	TARGETS zserv_cli 
				RUNTIME DESTINATION bin
				LIBRARY DESTINATION lib)

	install( 	TARGETS modchmlib
				RUNTIME DESTINATION bin
				LIBRARY DESTINATION lib)

	set(CPACK_PACKAGE_CONTACT "bigplayer.dev@gmail.com")
	set(CPACK_DEBIAN_PACKAGE_MAINTAINER "zoltan fabian")
	set(CPACK_DEBIAN_PACKAGE_DEPENDS zlib1g)

	string(TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_PACKAGE_NAME_LOWERCASE)
	find_program(DPKG_PROGRAM dpkg DOC "dpkg program of Debian-based systems")
	if(DPKG_PROGRAM)
	  execute_process(
		COMMAND ${DPKG_PROGRAM} --print-architecture
		OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
		OUTPUT_STRIP_TRAILING_WHITESPACE
	  )
	  set(CPACK_PACKAGE_FILE_NAME
	"${CPACK_PACKAGE_NAME_LOWERCASE}-${MY_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")   

	else(DPKG_PROGRAM)
	  set(CPACK_PACKAGE_FILE_NAME
	"${CPACK_PACKAGE_NAME_LOWERCASE}-${MY_VERSION}_${CMAKE_SYSTEM_NAME}")
	endif(DPKG_PROGRAM)


ENDIF(WIN32 AND NOT UNIX)

INCLUDE(CPack)

