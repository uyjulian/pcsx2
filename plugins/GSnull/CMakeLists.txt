# Check that people use the good file
if(NOT TOP_CMAKE_WAS_SOURCED)
    message(FATAL_ERROR "
    You did not 'cmake' the good CMakeLists.txt file. Use the one in the top dir.
    It is advice to delete all wrongly generated cmake stuff => CMakeFiles & CMakeCache.txt")
endif()


# plugin name
set(Output GSnull)

set(OptimizationFlags
    -O2
    )

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    set(GSnullFinalFlags "")

elseif(CMAKE_BUILD_TYPE STREQUAL Devel)
	set(GSnullFinalFlags ${OptimizationFlags})

elseif(CMAKE_BUILD_TYPE STREQUAL Release)
	set(GSnullFinalFlags ${OptimizationFlags})

endif()

# GSnull sources
set(GSnullSources
	GifTransfer.cpp
	GS.cpp
	GSLog.cpp
	Registers.cpp)

# GSnull headers
set(GSnullHeaders
	GifTransfer.h
	GS.h
	Registers.h)

# GSnull Linux (GTK) sources
set(GSnullLinuxSources
	Linux/Config.cpp
	Linux/GSLinux.cpp
	Linux/Linux.cpp)

# GSnull Linux (GTK) headers
set(GSnullLinuxHeaders
	Linux/Config.h
	Linux/GSLinux.h
	Linux/Linux.h)
	
# GSnull Wxwidgets sources
set(GSnullWxSources
	Linux/Config.cpp
	Wx/GSWx.cpp
	Linux/Linux.cpp)

# GSnull Wxwidgets headers
set(GSnullWxHeaders
	Linux/Config.h
	Wx/GSWx.h
	Linux/Linux.h)

# GSnull null sources
set(GSnullnullSources
	null/GSnull.cpp)

# GSnull null headers
set(GSnullnullHeaders
	null/GSnull.h)

# GSnull Windows sources
set(GSnullWindowsSources
	Windows/Config.cpp
	Windows/GS.def
	Windows/GS.rc
	Windows/GSwin.cpp
	Windows/Win32.cpp)

# GSnull Windows headers
set(GSnullWindowsHeaders
	)

if(APPLE)
	set(GSnullFinalSources
		${GSnullSources}
		${GSnullHeaders}
		${GSnullnullSources}
		${GSnullnullHeaders}
		${GSnullWxSources}
		${GSnullWxHeaders}
	)
#Todo add Windows here if needed
else()
	set(GSnullFinalSources
		${GSnullSources}
		${GSnullHeaders}
		${GSnullnullSources}
		${GSnullnullHeaders}
		${GSnullLinuxSources}
		${GSnullLinuxHeaders}
	)
endif()

set(GSnullFinalLibs
	${GTK2_LIBRARIES}
	${X11_LIBRARIES}
	${wxWidgets_LIBRARIES}
)

add_pcsx2_plugin(${Output} "${GSnullFinalSources}" "${GSnullFinalLibs}" "${GSnullFinalFlags}")
