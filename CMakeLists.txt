cmake_minimum_required(VERSION 3.16)
project(BePDF VERSION 2.1.5 LANGUAGES C CXX)

# BePDF for V\OS, built outside the Vitruvian tree against an installed
# vos-dev. Needs: vos-dev (headers, vos.pc), the vos runtime tools
# (rc, xres, resattr, linkcatkeys), libfreetype-dev and libicu-dev.

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

find_package(PkgConfig REQUIRED)
find_package(Freetype REQUIRED)
pkg_check_modules(VOS REQUIRED vos)

# BePDF uses four private headers (ColumnListView, ColumnTypes, ToolTip,
# ToolBar), installed under /system/develop/headers/private: by the vos
# package in the amd64 builds published so far, by vos-dev in newer ones.
set(VOS_PRIVATE_HEADERS "/system/develop/headers/private" CACHE PATH
	"Directory with the V\\OS private headers (interface/, shared/)")
if(NOT EXISTS "${VOS_PRIVATE_HEADERS}/interface/ColumnListView.h")
	message(FATAL_ERROR
		"${VOS_PRIVATE_HEADERS}/interface/ColumnListView.h not found: "
		"install vos and vos-dev or set -DVOS_PRIVATE_HEADERS=<dir>")
endif()

foreach(_tool rc xres resattr linkcatkeys)
	string(TOUPPER ${_tool} _var)
	find_program(VOS_${_var} ${_tool} REQUIRED)
endforeach()

# Everything that compiles against V\OS goes through this target.
add_library(vos_sdk INTERFACE)
add_library(vos::sdk ALIAS vos_sdk)
# Not SYSTEM, and listed before xpdf's own directories: xpdf has headers named
# like Be's (UTF8.h, Catalog.h) and the V\OS ones must win, as in-tree.
get_filename_component(_vos_headers_root "${VOS_PRIVATE_HEADERS}" DIRECTORY)
target_include_directories(vos_sdk INTERFACE
	${VOS_INCLUDE_DIRS}
	${_vos_headers_root}
	${VOS_PRIVATE_HEADERS}
	${VOS_PRIVATE_HEADERS}/interface
	${VOS_PRIVATE_HEADERS}/shared
)
target_compile_options(vos_sdk INTERFACE
	"SHELL:-include LinuxBuildCompatibility.h"
	-fPIC -Wno-multichar -Wno-deprecated -Wno-write-strings
	$<$<COMPILE_LANGUAGE:CXX>:-fpermissive>
	$<$<COMPILE_LANGUAGE:CXX>:-fexceptions>
)
target_link_directories(vos_sdk INTERFACE ${VOS_LIBRARY_DIRS})
# libroot first, kept in DT_NEEDED for its strerror (same as the in-tree
# Application() macro).
target_link_options(vos_sdk INTERFACE -Wl,--no-as-needed)
target_link_libraries(vos_sdk INTERFACE
	localestub tracker translation textencoding
	be root
	icui18n icuuc icudata icuio
)

# Put the V\OS include directories ahead of the target's other ones.
function(vos_includes_first target)
	target_include_directories(${target} BEFORE PRIVATE
		$<TARGET_PROPERTY:vos_sdk,INTERFACE_INCLUDE_DIRECTORIES>)
endfunction()

add_subdirectory(xpdf)
add_subdirectory(src)
add_subdirectory(locales)

include(cmake/Packaging.cmake)
