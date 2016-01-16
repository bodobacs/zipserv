cmake_minimum_required(VERSION 2.4.4)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

project(chmlib)
set(VERSION "0.1")

#define CHM_DEBUG
add_definitions(-DCHM_DEBUG)

IF(UNIX)

	#locally builded library
	add_library(chmlib chm_lib.c lzx.c)

	#executable
	add_executable(main main.cpp)
	target_link_libraries(main chmlib)

	#executable
	add_executable(mytestchm list-chm.cpp)
	target_link_libraries(mytestchm chmlib)

ELSE() #IF(UNIX)
ENDIF(UNIX) #IF(UNIX)

