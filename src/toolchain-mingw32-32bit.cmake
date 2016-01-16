# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
#SET(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
SET(CMAKE_C_COMPILER i686-w64-mingw32-g++)
SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)

set(CMAKE_C_FLAGS -m32)
set(CMAKE_CXX_FLAGS -m32)

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static")
#set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic")

# here is the target environment located
#32bit LIBRARIES!!!!!
SET(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32 $ENV{HOME}/libsforwindows)

#64bit LIBRARIES!!!!!
#SET(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32 $ENV{HOME}/libsforwindows)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


#usage:
#cmake -DCMAKE_TOOLCHAIN_FILE=../src/toolchain-mingw32-32bit.cmake ../src/
