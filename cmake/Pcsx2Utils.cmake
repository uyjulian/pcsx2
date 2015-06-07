#-------------------------------------------------------------------------------
#						detectOperatingSystem
#-------------------------------------------------------------------------------
# This function detects on which OS cmake is run and set a flag to control the
# build process. Supported OS: Linux, MacOSX, Windows
# 
# On linux, it also set a flag for specific distribution (ie Fedora)
#-------------------------------------------------------------------------------
function(detectOperatingSystem)
    if(WIN32)
        set(Windows TRUE PARENT_SCOPE)
    elseif(UNIX AND APPLE)
        # No easy way to filter out iOS.
        message(WARNING "OS X/iOS isn't supported, the build will most likely fail")
        set(MacOSX TRUE PARENT_SCOPE)
    elseif(UNIX)
        if(CMAKE_SYSTEM_NAME MATCHES "Linux")
            set(Linux TRUE PARENT_SCOPE)
            if (EXISTS /etc/os-release)
                # Read the file without CR character
                file(STRINGS /etc/os-release OS_RELEASE)
                if("${OS_RELEASE}" MATCHES "^.*ID=fedora.*$")
                    set(Fedora TRUE PARENT_SCOPE)
                    message(STATUS "Build Fedora specific")
                elseif("${OS_RELEASE}" MATCHES "^.*ID=.*suse.*$")
                    set(openSUSE TRUE PARENT_SCOPE)
                    add_definitions(-DopenSUSE)
                    message(STATUS "Build openSUSE specific")
                endif()
            endif()
        elseif(CMAKE_SYSTEM_NAME MATCHES "kFreeBSD")
            set(kFreeBSD TRUE PARENT_SCOPE)
        elseif(CMAKE_SYSTEM_NAME STREQUAL "GNU")
            set(GNU TRUE PARENT_SCOPE)
        endif()
    endif()
endfunction()

function(write_svnrev_h)
    set(PCSX2_WC_TIME 0)
    if (GIT_FOUND AND EXISTS ${PROJECT_SOURCE_DIR}/.git)
        EXECUTE_PROCESS(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} show -s --format=%ci HEAD
            OUTPUT_VARIABLE PCSX2_WC_TIME
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        # Output: "YYYY-MM-DD HH:MM:SS +HHMM" (last part is time zone, offset from UTC)
        string(REGEX REPLACE "[%:\\-]" "" PCSX2_WC_TIME "${PCSX2_WC_TIME}")
        string(REGEX REPLACE "([0-9]+) ([0-9]+).*" "\\1\\2" PCSX2_WC_TIME "${PCSX2_WC_TIME}")
    endif()
    if ("${PCSX2_WC_TIME}" STREQUAL "")
        set(PCSX2_WC_TIME 0)
    endif()
    file(WRITE ${CMAKE_BINARY_DIR}/common/include/svnrev.h "#define SVN_REV ${PCSX2_WC_TIME}ll \n#define SVN_MODS 0")
endfunction()

function(check_compiler_version version_warn version_err)
    if(CMAKE_COMPILER_IS_GNUCXX)
        execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
        string(STRIP "${GCC_VERSION}" GCC_VERSION)
        if(GCC_VERSION VERSION_LESS ${version_err})
            message(FATAL_ERROR "PCSX2 doesn't support your old GCC ${GCC_VERSION}! Please upgrade it!
            
            The minimum supported version is ${version_err} but ${version_warn} is warmly recommended")
        else()
            if(GCC_VERSION VERSION_LESS ${version_warn})
                message(WARNING "PCSX2 will stop supporting GCC ${GCC_VERSION} in the near future. Please upgrade to at least GCC ${version_warn}.")
            endif()
        endif()
    endif()
endfunction()

function(check_no_parenthesis_in_path)
    if ("${CMAKE_BINARY_DIR}" MATCHES "[()]" OR "${CMAKE_SOURCE_DIR}" MATCHES "[()]")
        message(FATAL_ERROR "Your path contains some parenthesis. Unfortunately Cmake doesn't support them correctly.\nPlease rename your directory to avoid '(' and ')' characters\n")
    endif()
endfunction()

#NOTE: this macro is used to get rid of whitespace and newlines.
macro(append_flags target flags)
    if(flags STREQUAL "")
        set(flags " ") # set to space to avoid error
    endif()
    get_target_property(TEMP ${target} COMPILE_FLAGS)
    if(TEMP STREQUAL "TEMP-NOTFOUND")
        set(TEMP "") # set to empty string
    else()
        set(TEMP "${TEMP} ") # a space to cleanly separate from existing content
    endif()
    # append our values
    set(TEMP "${TEMP}${flags}")
    # fix arg list
    set(TEMP2 "")
    foreach(_arg ${TEMP})
        set(TEMP2 "${TEMP2} ${_arg}")
    endforeach()
    set_target_properties(${target} PROPERTIES COMPILE_FLAGS "${TEMP2}")
endmacro(append_flags)

macro(add_pcsx2_plugin lib srcs libs flags)
    include_directories(.)

    # TODO OSX do we want to use .so libs on OSX or .dylib? http://www.cmake.org/pipermail/cmake/2011-September/046098.html
    if(APPLE)
        #CMAKE_SHARED_MODULE_SUFFIX
        add_library(${lib} SHARED ${srcs})
    else()
        add_library(${lib} MODULE ${srcs})
    endif()
    target_link_libraries(${lib} ${libs})
    append_flags(${lib} "${flags}")
    if(NOT USER_CMAKE_LD_FLAGS STREQUAL "")
        target_link_libraries(${lib} "${USER_CMAKE_LD_FLAGS}")
    endif(NOT USER_CMAKE_LD_FLAGS STREQUAL "")
    if(PACKAGE_MODE)
        install(TARGETS ${lib} DESTINATION ${PLUGIN_DIR})
    else(PACKAGE_MODE)
        install(TARGETS ${lib} DESTINATION ${CMAKE_SOURCE_DIR}/bin/plugins)
    endif(PACKAGE_MODE)
endmacro(add_pcsx2_plugin)

macro(add_pcsx2_lib lib srcs libs flags)
    include_directories(.)
    add_library(${lib} STATIC ${srcs})
    target_link_libraries(${lib} ${libs})
    append_flags(${lib} "${flags}")
    if(NOT USER_CMAKE_LD_FLAGS STREQUAL "")
        target_link_libraries(${lib} "${USER_CMAKE_LD_FLAGS}")
    endif(NOT USER_CMAKE_LD_FLAGS STREQUAL "")
endmacro(add_pcsx2_lib)

macro(add_pcsx2_executable exe srcs libs flags)
    add_definitions(${flags})
    include_directories(.)
    add_executable(${exe} ${srcs})
    target_link_libraries(${exe} ${libs})
    append_flags(${exe} "${flags}")
    if(NOT USER_CMAKE_LD_FLAGS STREQUAL "")
        target_link_libraries(${lib} "${USER_CMAKE_LD_FLAGS}")
    endif(NOT USER_CMAKE_LD_FLAGS STREQUAL "")
    if(PACKAGE_MODE)
        install(TARGETS ${exe} DESTINATION ${BIN_DIR})
    else(PACKAGE_MODE)
        install(TARGETS ${exe} DESTINATION ${CMAKE_SOURCE_DIR}/bin)
    endif(PACKAGE_MODE)
endmacro(add_pcsx2_executable)
