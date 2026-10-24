set(_sources
	AnnotAppearance.cpp
	AnnotWriter.cpp
	Annotation.cpp
	AnnotationRenderer.cpp
	AnnotationWindow.cpp
	AttachmentView.cpp
	BeFontEncoding.cpp
	BePDF.cpp
	BepdfApplication.cpp
	BitmapPool.cpp
	CachedPage.cpp
	DisplayCIDFonts.cpp
	EntryChangedMonitor.cpp
	FileInfoWindow.cpp
	FileSpec.cpp
	FindTextWindow.cpp
	History.cpp
	LayoutUtils.cpp
	OutlinesWindow.cpp
	PDFPrint.cpp
	PDFSearch.cpp
	PDFView.cpp
	PDFWindow.cpp
	PageCache.cpp
	PageLabels.cpp
	PageRenderer.cpp
	PageThumbnails.cpp
	PasswordWindow.cpp
	PreferencesWindow.cpp
	PrintSettingsWindow.cpp
	PrintingProgressWindow.cpp
	SaveThread.cpp
	Settings.cpp
	StatusWindow.cpp
	TextConversion.cpp
	TraceWindow.cpp
	TreeParser.cpp
	utils/EntryMenuItem.cpp
	utils/ResourceLoader.cpp
	utils/Thread.cpp
)

add_executable(BePDF ${_sources})
target_include_directories(BePDF PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(BePDF PRIVATE __HAIKU__)
target_link_libraries(BePDF PRIVATE vos::sdk xpdf)
vos_includes_first(BePDF)

# Resources: preprocess the rdef (as the in-tree build does, so it may use
# #ifdef), compile it, embed it and turn it into BeOS attributes. The same
# attributes go into attrs.tar, which the package's postinst restores onto
# the installed binary: dpkg does not carry xattrs.
set(_rdef  "${CMAKE_CURRENT_SOURCE_DIR}/BePDF.rdef")
set(_pp    "${CMAKE_CURRENT_BINARY_DIR}/BePDF.rdef.pp")
set(_rsrc  "${CMAKE_CURRENT_BINARY_DIR}/BePDF.rdef.rsrc")
set(_attrs "${CMAKE_BINARY_DIR}/attrs")

add_custom_command(TARGET BePDF POST_BUILD
	COMMENT "Embedding resources and attributes into BePDF"
	COMMAND "${CMAKE_C_COMPILER}" -E -x c -DHAIKU_TARGET_PLATFORM_HAIKU
		-D__VOS__ -P "${_rdef}" -o "${_pp}"
	COMMAND "${VOS_RC}" "${_pp}" -o "${_rsrc}"
	COMMAND "${VOS_XRES}" -o "$<TARGET_FILE:BePDF>" "${_rsrc}"
	COMMAND "${VOS_RESATTR}" -O -o "$<TARGET_FILE:BePDF>" "${_rsrc}"
	COMMAND ${CMAKE_COMMAND} -E make_directory "${_attrs}/system/apps"
	COMMAND ${CMAKE_COMMAND} -E touch "${_attrs}/system/apps/BePDF"
	COMMAND "${VOS_RESATTR}" -O -o "${_attrs}/system/apps/BePDF" "${_rsrc}"
	COMMAND tar --xattrs -cf "${CMAKE_BINARY_DIR}/attrs.tar" -C "${_attrs}" .
	DEPENDS "${_rdef}"
)
set_property(TARGET BePDF APPEND PROPERTY ADDITIONAL_CLEAN_FILES
	"${_pp}" "${_rsrc}" "${CMAKE_BINARY_DIR}/attrs.tar")
