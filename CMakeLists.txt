# CMakeList.txt : CMake project for MPML, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.20)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (
	FILL
	VERSION 0.1.00
	LANGUAGES CXX
)


# Make library

add_library(FILL
	"${PROJECT_SOURCE_DIR}/include/image.hpp"
	"${PROJECT_SOURCE_DIR}/src/image.cpp"
)

add_library(FILL::FILL ALIAS FILL)



target_include_directories(FILL
	PUBLIC

		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		
		$<INSTALL_INTERFACE:include>
)

target_compile_features(FILL PUBLIC cxx_std_20)


# Dowloading Rules

include(GNUInstallDirs)


install(
	TARGETS FILL
	EXPORT FILLtargets
)

install(
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
	EXPORT FILLtargets
	NAMESPACE FILL::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FILL
)
