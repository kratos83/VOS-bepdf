# Catalogs: one .catkeys per language, signature x-vnd.mp-xpdf (the app's
# signature without the "application/" prefix).
set(_signature "x-vnd.mp-xpdf")
set(_catdir "${CMAKE_BINARY_DIR}/catalogs/${_signature}")

file(GLOB _catkeys "${CMAKE_CURRENT_SOURCE_DIR}/*.catkeys")
set(_catalogs)
foreach(_file ${_catkeys})
	get_filename_component(_lang ${_file} NAME_WE)
	set(_out "${_catdir}/${_lang}.catalog")
	add_custom_command(
		OUTPUT "${_out}"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${_catdir}"
		COMMAND "${VOS_LINKCATKEYS}" -s "${_signature}" -l "${_lang}" -tf
			-o "${_out}" "${_file}"
		DEPENDS "${_file}"
		COMMENT "Building catalog ${_signature}/${_lang}"
	)
	list(APPEND _catalogs "${_out}")
endforeach()
add_custom_target(catalogs ALL DEPENDS ${_catalogs})

install(DIRECTORY "${_catdir}/"
	DESTINATION "/system/data/locale/catalogs/${_signature}"
	FILES_MATCHING PATTERN "*.catalog")
