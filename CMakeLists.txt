cmake_minimum_required(VERSION 3.1)

# cmake config
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# this line has to appear before 'PROJECT' in order to be able to disable incremental linking
set(MSVC_INCREMENTAL_DEFAULT ON)

# begin project
project(SDKMESH_EXPORTER)

# output
set(LIBRARY_OUTPUT_PATH ${SDKMESH_EXPORTER_SOURCE_DIR}/Bin/${CMAKE_SYSTEM_NAME}/Libs CACHE PATH "Single output directory for building all libraries.")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SDKMESH_EXPORTER_SOURCE_DIR}/Bin CACHE PATH "Single output directory for building executables.")

# debug/release
if (NOT CMAKE_BUILD_TYPE)
	# set(CMAKE_BUILD_TYPE "Debug")
	set(CMAKE_BUILD_TYPE "Release")
endif()

set (APP_VERSION "1.0.0")

message(STATUS "- System: ${CMAKE_SYSTEM_NAME} - BuildType: ${CMAKE_BUILD_TYPE}")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
endif()

set(CMAKE_CXX_STANDARD 11)

option(USE_MSVC_RUNTIME_LIBRARY_DLL "Use MSVC Runtime Library DLL (/MD or /MDd)" ON)

# unicode library
add_definitions(-DUNICODE -D_UNICODE)	

if (CYGWIN)
	add_definitions(-DCYGWIN)
endif()

if (MINGW)
	add_definitions(-DMINGW)
endif()

# visual studio
if (MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)

	if (NOT USE_MSVC_RUNTIME_LIBRARY_DLL)
		#We statically link to reduce dependancies
		foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
			if (${flag_var} MATCHES "/MD")
				string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
			endif (${flag_var} MATCHES "/MD")
			if (${flag_var} MATCHES "/MDd")
				string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
			endif (${flag_var} MATCHES "/MDd")
		endforeach()
	endif()
		
	if (CMAKE_CL_64)
		add_definitions(-D_WIN64)
	endif()
	
	option(USE_MSVC_FAST_FLOATINGPOINT "Use MSVC /fp:fast option" ON)
	if (USE_MSVC_FAST_FLOATINGPOINT)
		  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:fast")
	endif()

	option(USE_MSVC_STRING_POOLING "Use MSVC /GF string pooling option"	ON)
	if (USE_MSVC_STRING_POOLING)
		set(CMAKE_C_FLAGS "/GF ${CMAKE_C_FLAGS}")
		set(CMAKE_CXX_FLAGS "/GF ${CMAKE_CXX_FLAGS}")
	endif()

	option(USE_MSVC_FUNCTION_LEVEL_LINKING "Use MSVC /Gy function level linking option"	ON)
	if (USE_MSVC_FUNCTION_LEVEL_LINKING)
		set(CMAKE_C_FLAGS "/Gy ${CMAKE_C_FLAGS}")
		set(CMAKE_CXX_FLAGS "/Gy ${CMAKE_CXX_FLAGS}")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /OPT:REF")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /OPT:REF")
	endif()
	
	option(USE_MSVC_COMDAT_FOLDING "Use MSVC /OPT:ICF COMDAT folding option" ON)
	if (USE_MSVC_COMDAT_FOLDING)
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /OPT:ICF")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /OPT:ICF")
	endif()

	set(CMAKE_EXE_LINKER_FLAGS	"${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
endif()

# core library
include_directories(
	${SDKMESH_EXPORTER_SOURCE_DIR}/Source
)

file(GLOB_RECURSE main_app_source 
	./Source/**.cpp 
	./Source/**.c 
	./Source/**.h)

add_executable(SDKMeshObjExporter ${main_app_source})	

set_target_properties(SDKMeshObjExporter PROPERTIES VERSION ${APP_VERSION})
set_target_properties(SDKMeshObjExporter PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")