find_package(ZLIB REQUIRED)

add_library(FILL
	include/image.hpp
	src/image.cpp
)

add_library(FILL::FILL ALIAS FILL)

target_include_directories(FILL
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

target_compile_features(FILL PUBLIC cxx_std_20)

target_link_libraries(FILL
	PUBLIC ZLIB::ZLIB
)

install(TARGETS FILL
	EXPORT FILLtargets
)

install(DIRECTORY include/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT FILLtargets
	NAMESPACE FILL::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FILL
)



