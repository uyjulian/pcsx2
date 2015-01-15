# Tell cmake we are cross compiling and targeting darwin
# TODO OSX below is not proper and causes errors. CMAKE_???_COMPILER_TARGET alone is just fine.
#set(CMAKE_SYSTEM_NAME Darwin)
#set(CMAKE_SYSTEM_PROCESSOR i686)

# TODO OSX Lion/Snow Lion target support might be possible also?
# Use clang and target i686-apple-darwin13.0.0 (Mavericks)
set(CMAKE_C_COMPILER clang -m32)
set(CMAKE_C_COMPILER_TARGET i686-apple-darwin13.0.0)
set(CMAKE_CXX_COMPILER clang++ -m32)
set(CMAKE_CXX_COMPILER_TARGET i686-apple-darwin13.0.0)


# If given a CMAKE_FIND_ROOT_PATH then
# FIND_PROGRAM ignores CMAKE_FIND_ROOT_PATH (probably can't run)
# FIND_{LIBRARY,INCLUDE,PACKAGE} only uses the files in CMAKE_FIND_ROOT_PATH.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
