cmake_minimum_required(VERSION 3.21)


project(AutoMove VERSION 0.3.0.0 LANGUAGES CXX)
set(PROJECT_PLUGIN_NAME "AutoMove NG")


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

include(GNUInstallDirs)

# CommonLibSSE-NG (alandtse fork) is a git submodule in lib/commonlibsse-ng.
# Build for flat Skyrim only: SE 1.5.97 and AE 1.6.x / 1.7.x. VR is not supported.
set(ENABLE_SKYRIM_SE ON CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_AE ON CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_VR OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/commonlibsse-ng EXCLUDE_FROM_ALL)
include(lib/commonlibsse-ng/cmake/CommonLibSSE.cmake)




# Do not delete the below begin and end statements.
# They are used by the globber py script for replacement
### BEGIN SOURCE FILES ###
set(SOURCES
	src/CustomDestination.cpp
	src/main.cpp
	src/Papyrus.cpp
	src/PCH.cpp
	src/Serialization.cpp
	src/Events/Events.cpp
	src/Events/OnCustomDestinationArrived.cpp
	src/Events/OnMarkerChange.cpp
	src/Events/OnPlayerDialogue.cpp
	src/Hooks/AutoMoveHook.cpp
	src/Hooks/CameraHooks.cpp
	src/Hooks/Hooks.cpp
)

set(HEADERS
	src/CustomDestination.h
	src/Papyrus.h
	src/PCH.h
	src/Serialization.h
	src/Events/Events.h
	src/Events/OnCustomDestinationArrived.h
	src/Events/OnMarkerChange.h
	src/Events/OnPlayerDialogue.h
	src/Hooks/Hooks.h
)
### END SOURCE FILES ###

add_commonlibsse_plugin(${PROJECT_NAME}
    # The plugin's name, defaults to target.
    NAME "${PROJECT_PLUGIN_NAME}"

    # The plugin's author, empty by default.
    AUTHOR "KAOS (a.k.a. xILARTH)"

    # The support email address, empty by default.
    EMAIL "kaos4d1@gmail.com"

    # Indicates the plugin is compatible with all runtimes via address library. This is the default if no
    # other compatibilility mode is specified. Can be used with USE_SIGNATURE_SCANNING but not
    # COMPATIBLE_RUNTIMES.
    USE_ADDRESS_LIBRARY

    # Indicates the plugin is compatible with all runtimes via signature scanning.  Can be used with
    # USE_ADDRESS_LIBRARY but not COMPATIBLE_RUNTIMES.
    USE_SIGNATURE_SCANNING

    # List of the sources to include in the target, as would be the parameters to add_library.
    SOURCES ${SOURCES} ${HEADERS}
)

target_compile_features(
    ${PROJECT_NAME}
    PUBLIC
        cxx_std_23
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
endif()

target_precompile_headers(
    ${PROJECT_NAME}
    PRIVATE
        src/PCH.h
)


# Copy the dll into Mod Organizer only when MO_SSE_INSTANCE is set (not in CI).
if(DEFINED ENV{MO_SSE_INSTANCE})
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "$ENV{MO_SSE_INSTANCE}/mods/${PROJECT_PLUGIN_NAME}/SKSE/Plugins/"
    )
endif()

find_package(Python3 COMPONENTS Interpreter)
install(CODE "execute_process(COMMAND \"${Python3_EXECUTABLE}\" \"${CMAKE_CURRENT_SOURCE_DIR}/scripts/mkrelease.py\" -v ${PROJECT_VERSION} -n \"${PROJECT_PLUGIN_NAME}\" -f \"$<TARGET_FILE:${PROJECT_NAME}>\" -o \"${PROJECT_NAME}\")")
