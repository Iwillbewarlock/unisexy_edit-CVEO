cmake_minimum_required(VERSION 3.21)
set(NAME "ActionMagic" CACHE STRING "")
set(VERSION 0.1.0 CACHE STRING "")

option(COPY_BUILD "Copy the build output to the Skyrim directory." FALSE)

macro(set_from_environment VARIABLE)
	if (NOT DEFINED ${VARIABLE} AND DEFINED ENV{${VARIABLE}})
		set(${VARIABLE} $ENV{${VARIABLE}})
	endif ()
endmacro()

set_from_environment(VCPKG_ROOT)
set_from_environment(Skyrim64Path)

# Same convention as the parent Unisexy project: the AE definitions are always on,
# so the plugin exports SKSEPlugin_Version only and runs on SE (via Address Library) and AE.
add_compile_definitions(SKYRIM_AE)
add_compile_definitions(SKYRIM_SUPPORT_AE)

# Reuse the parent repository's CommonLibSSE submodule (powerof3, dev branch), or CommonLibSSEPath.
find_path(CommonLibPath
	include/REL/Relocation.h
	PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../extern/CommonLibSSE
	NO_DEFAULT_PATH)
if (${CommonLibPath} STREQUAL "CommonLibPath-NOTFOUND")
	set_from_environment(CommonLibSSEPath)
	set(CommonLibPath ${CommonLibSSEPath})
endif ()

if (DEFINED VCPKG_ROOT)
	set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
	set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
else ()
	message(WARNING "Variable VCPKG_ROOT is not set. Continuing without vcpkg.")
endif ()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "")

project(
	${NAME}
	VERSION ${VERSION}
	LANGUAGES CXX
)

if (PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
	message(FATAL_ERROR "In-source builds not allowed. Use a separate build directory.")
endif ()

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/Version.h.in
	${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
	@ONLY
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
	@ONLY
)

if (MSVC AND NOT ${CMAKE_GENERATOR} STREQUAL "Ninja")
	add_compile_options(/MP)
endif ()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

# ---- Dependencies ----

if (DEFINED CommonLibPath AND NOT "${CommonLibPath}" STREQUAL "" AND IS_DIRECTORY ${CommonLibPath})
	add_subdirectory(${CommonLibPath} CommonLibSSE)
else ()
	message(FATAL_ERROR "CommonLibSSE not found. Run 'git submodule update --init' in the repository root or set CommonLibSSEPath.")
endif ()

find_path(SIMPLEINI_INCLUDE_DIRS "SimpleIni.h")

# ---- Sources ----

include(cmake/headerlist.cmake)
include(cmake/sourcelist.cmake)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${headers} ${sources})

add_library(
	${PROJECT_NAME}
	SHARED
	${headers}
	${sources}
	${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
	.clang-format
	.editorconfig
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_compile_definitions(${PROJECT_NAME} PRIVATE _UNICODE)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${SIMPLEINI_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE CommonLibSSE::CommonLibSSE)
target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)

if (MSVC)
	target_compile_options(
		${PROJECT_NAME}
		PRIVATE
			/sdl
			/utf-8
			/Zi
			/permissive-
			/Zc:preprocessor
			/wd4200
			"$<$<CONFIG:RELEASE>:/Zc:inline;/JMC-;/Ob3>"
	)

	target_link_options(
		${PROJECT_NAME}
		PRIVATE
			"$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
			"$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
	)
endif ()

if (COPY_BUILD)
	if (DEFINED Skyrim64Path)
		add_custom_command(
			TARGET ${PROJECT_NAME}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${Skyrim64Path}/Data/SKSE/Plugins/
			COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${PROJECT_NAME}> ${Skyrim64Path}/Data/SKSE/Plugins/
		)
	else ()
		message(WARNING "Skyrim64Path is not defined. Skipping post-build copy.")
	endif ()
endif ()
