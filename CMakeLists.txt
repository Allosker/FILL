# CMakeList.txt : CMake project for FILL, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("FILL")

# Create paths to file folders
set(SRC_DIR "${PROJECT_SOURCE_DIR}/src/")
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/")



# Add source and include to this project's executable.
file(GLOB_RECURSE SRC_FILES 
	"${SRC_DIR}/*.cpp"
	"${SRC_DIR}/*.c"
)

# Seek all files (recursively)
file(GLOB_RECURSE INCLUDE_FILES CMAKE_CONFIGURE_DEPENDS
	"${INCLUDE_DIR}/*.hpp"
	"${INCLUDE_DIR}/*.h"
)

add_executable( FILL
	${SRC_FILES}
	${INCLUDE_FILES}
) 

target_include_directories(FILL PUBLIC ${INCLUDE_DIR})


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET FILL PROPERTY CXX_STANDARD 23)
endif()

set(FILE_DIR "${CMAKE_SOURCE_DIR}/src/")
target_compile_definitions(FILL PRIVATE FILE_PATH="${FILE_DIR}") 

# TODO: Add tests and install targets if needed.

include(FetchContent)
 
FetchContent_Declare(
	zlibstatic
	GIT_REPOSITORY	https://github.com/madler/zlib.git
	#GIT_TAG 	fb375dd8c0a032106a2122815fb18dffe0283721 #refs/tags/v6.0.2
)


FetchContent_MakeAvailable(zlibstatic)
 

 
target_link_libraries(FILL PRIVATE zlibstatic)