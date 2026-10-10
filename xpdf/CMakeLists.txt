# xpdf 4.0 as patched for BePDF, built as a static library.

add_library(xpdf STATIC
	beos/BeSplashOutputDev.cc
	beos/EncodingReader.cc
	beos/Init.cc
	beos/GDir.cc

	fofi/FoFiBase.cc
	fofi/FoFiEncodings.cc
	fofi/FoFiIdentifier.cc
	fofi/FoFiTrueType.cc
	fofi/FoFiType1.cc
	fofi/FoFiType1C.cc

	goo/parseargs.c
	goo/FixedPoint.cc
	goo/GHash.cc
	goo/GList.cc
	goo/GString.cc
	goo/gfile.cc
	goo/gmem.cc
	goo/gmempp.cc

	splash/Splash.cc
	splash/SplashBitmap.cc
	splash/SplashClip.cc
	splash/SplashFTFont.cc
	splash/SplashFTFontEngine.cc
	splash/SplashFTFontFile.cc
	splash/SplashFont.cc
	splash/SplashFontEngine.cc
	splash/SplashFontFile.cc
	splash/SplashFontFileID.cc
	splash/SplashPath.cc
	splash/SplashPattern.cc
	splash/SplashScreen.cc
	splash/SplashState.cc
	splash/SplashXPath.cc
	splash/SplashXPathScanner.cc

	xpdf/AcroForm.cc
	xpdf/Annot.cc
	xpdf/Array.cc
	xpdf/BuiltinFont.cc
	xpdf/BuiltinFontTables.cc
	xpdf/CMap.cc
	xpdf/Catalog.cc
	xpdf/CharCodeToUnicode.cc
	xpdf/Decrypt.cc
	xpdf/Dict.cc
	xpdf/DisplayState.cc
	xpdf/Error.cc
	xpdf/FontEncodingTables.cc
	xpdf/Form.cc
	xpdf/Function.cc
	xpdf/Gfx.cc
	xpdf/GfxFont.cc
	xpdf/GfxState.cc
	xpdf/GlobalParams.cc
	xpdf/ImageOutputDev.cc
	xpdf/JArithmeticDecoder.cc
	xpdf/JBIG2Stream.cc
	xpdf/JPXStream.cc
	xpdf/Lexer.cc
	xpdf/Link.cc
	xpdf/NameToCharCode.cc
	xpdf/Object.cc
	xpdf/OptionalContent.cc
	xpdf/Outline.cc
	xpdf/OutputDev.cc
	xpdf/PDFCore.cc
	xpdf/PDFDoc.cc
	xpdf/PDFDocEncoding.cc
	xpdf/PSOutputDev.cc
	xpdf/PSTokenizer.cc
	xpdf/Page.cc
	xpdf/Parser.cc
	xpdf/PreScanOutputDev.cc
	xpdf/SecurityHandler.cc
	xpdf/SplashOutputDev.cc
	xpdf/Stream.cc
	xpdf/TextOutputDev.cc
	xpdf/TextString.cc
	xpdf/TileCache.cc
	xpdf/TileCompositor.cc
	xpdf/TileMap.cc
	xpdf/UnicodeMap.cc
	xpdf/UnicodeTypeTable.cc
	xpdf/UTF8.cc
	xpdf/XFAForm.cc
	xpdf/XRef.cc
	xpdf/Zoox.cc
)

target_include_directories(xpdf PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/beos
	${CMAKE_CURRENT_SOURCE_DIR}/fofi
	${CMAKE_CURRENT_SOURCE_DIR}/goo
	${CMAKE_CURRENT_SOURCE_DIR}/splash
	${CMAKE_CURRENT_SOURCE_DIR}/xpdf
)

# xpdf picks its Haiku code paths (atomics, the Nimbus font table, public
# parameter tables) with __HAIKU__, which the V\OS toolchain does not define.
target_compile_definitions(xpdf PUBLIC __HAIKU__)

target_link_libraries(xpdf PUBLIC vos::sdk Freetype::Freetype)
vos_includes_first(xpdf)
