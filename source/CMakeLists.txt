cmake_minimum_required(VERSION 3.0) # I had problems if I only specified "VERSION 3".

project(Stub)

# Enables structuring of project into separate folders (e.g. in MSVC, allowing the existence of an INSTALL project).
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_path(COMPONENTS_ROOT PATHS NO_DEFAULT_PATH)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${COMPONENTS_ROOT}/cmake-modules")
set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "" FORCE)

include(ListSubdirectories)
include(TargetImportedLibraries) # for Walter's target_package command

###################################################################################################
# We use an EXTERNAL_LIBRARY_DIR variable as a hint to where all the libraries can be found.
# This is an optional means to not have to specify each library's root dir directly.
###################################################################################################

find_path(EXTERNAL_LIBRARY_DIR "glew-1.9.0" HINTS /opt/local/Libraries PATHS $ENV{PATH} $ENV{EXTERNAL_LIBRARY_DIR} NO_DEFAULT_PATH)

# TODO: Make EXTERNAL_LIBRARY_DIR detection optional, since users may not have their libraries
# installed the same way we (Leap) do.
if(EXTERNAL_LIBRARY_DIR STREQUAL "EXTERNAL_LIBRARY_DIR-NOTFOUND")
    message(STATUS "External Library Directory not found, please specify a folder to look for external libraries")
    return()
endif()

# CMAKE_PREFIX_PATH is the path used for searching by FIND_XXX(), with appropriate suffixes added.
# EXTERNAL_LIBRARY_DIR is a hint for all the find_library calls.
list(INSERT CMAKE_PREFIX_PATH 0 ${EXTERNAL_LIBRARY_DIR})

###################################################################################################
# Definition of how to build the Stub app
###################################################################################################

set(Stub_SOURCES
    main.cpp
    StubApplication.cpp
    StubApplication.h
    ExampleLayer.cpp
    ExampleLayer.h
)

set(_add_executable_options "")
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # This is the correct way to detect Mac OS X operating system -- see http://www.openguru.com/2009/04/cmake-detecting-platformoperating.html
    set(_add_executable_options "MACOSX_BUNDLE")
endif()

add_executable(Stub ${_add_executable_options} ${Stub_SOURCES})

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # This is the correct way to detect Mac OS X operating system -- see http://www.openguru.com/2009/04/cmake-detecting-platformoperating.html
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wall -Werror")
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /NODEFAULTLIB:LIBCMT")
endif()

find_package(Components)
# TODO: somehow get these library dependencies from the Components target
# Please keep these in alphabetical order.
find_package(Eigen 3.1.2)
find_package(FbxSdk)
find_package(FreeImage 3)
find_package(Glew 1.9.0)
find_package(OpenGL 2.1)
find_package(SDL 2)

target_imported_libraries(Stub PUBLIC SDL::SDL)
target_link_libraries(Stub PUBLIC Components::Components)

###################################################################################################
# Resource rules
###################################################################################################

set(RESOURCE_FILES
    public_domain_astronomy_1.jpg
)

# TODO: install steps (or resource-copying steps) for other platforms
# TODO: really it would be good to make a cmake module for how to install files in a cross-platform way.
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # This is the correct way to detect Mac OS X operating system -- see http://www.openguru.com/2009/04/cmake-detecting-platformoperating.html
    set(ACTUAL_BUILD_DIR "${PROJECT_BINARY_DIR}")
    if(${CMAKE_GENERATOR} MATCHES "Xcode")
        # CMAKE_BUILD_TYPE will be one of Release, Debug, etc.
        set(ACTUAL_BUILD_DIR "${ACTUAL_BUILD_DIR}/${CMAKE_BUILD_TYPE}")
    endif()
    # TODO: apparently there is a different "correct" way to install files on Mac;
    # see: http://www.cmake.org/cmake/help/v3.0/prop_sf/MACOSX_PACKAGE_LOCATION.html
    # http://www.cmake.org/cmake/help/v3.0/prop_tgt/RESOURCE.html
    # http://www.cmake.org/Wiki/CMake:Bundles_And_Frameworks#Bundle_only
    install(FILES ${RESOURCE_FILES} DESTINATION "${ACTUAL_BUILD_DIR}/Stub.app/Contents/Resources")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # TODO: figure out how this should be done correctly -- do we want to use the POSIX directory scheme, or what?
    # For now, just install into the binary dir, because that's where the SDL_GetBasePath will point us to.
    install(FILES ${RESOURCE_FILES} DESTINATION ${PROJECT_BINARY_DIR})
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        # CMAKE_BUILD_TYPE will be one of Release, Debug, etc.
    set(ACTUAL_BUILD_DIR "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
    install(FILES ${RESOURCE_FILES} DESTINATION "${ACTUAL_BUILD_DIR}")
endif()
