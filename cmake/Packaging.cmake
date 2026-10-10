# Installs into /system like the rest of V\OS, and packages as vos-bepdf.

install(TARGETS BePDF RUNTIME DESTINATION /system/apps)

# Fonts, encodings, license texts and the start document.
install(DIRECTORY "${CMAKE_SOURCE_DIR}/data/"
	DESTINATION /system/data/bepdf)

# BeOS attributes of the binary, restored by postinst (see src/CMakeLists.txt).
install(FILES "${CMAKE_BINARY_DIR}/attrs.tar"
	DESTINATION /usr/share/vos-bepdf)

execute_process(COMMAND dpkg --print-architecture
	OUTPUT_VARIABLE _arch OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
if(NOT _arch)
	set(_arch amd64)
endif()

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_NAME vos-bepdf)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "BePDF")
set(CPACK_PACKAGE_CONTACT "Angelo Scarnà <calang83@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_CONTACT}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "BePDF PDF reader for V\\\\OS")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION
	"PDF reader based on xpdf 4.0 with annotations, bookmarks, search and printing.")
set(CPACK_DEBIAN_PACKAGE_SECTION graphics)
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${_arch})
set(CPACK_DEBIAN_PACKAGE_DEPENDS "vos, vos-data, libfreetype6, attr")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
	"${CMAKE_SOURCE_DIR}/packaging/postinst"
	"${CMAKE_SOURCE_DIR}/packaging/postrm")

include(CPack)
