cmake_minimum_required(VERSION 3.0) # I had problems if I only specified "VERSION 3".

project(Space)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")
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

OPTION(OCULUS_SDK "Use Oculus Rift SDK" OFF)
if(OCULUS_SDK)
  ADD_DEFINITIONS(-DOCULUS_SDK)
  # Please keep these in alphabetical order.
  find_package(OculusSDK REQUIRED)

  if(WIN32)
    # Where to find Oculus SDK
	SET(OCULUS_SDK_ROOT_DIR $ENV{OCULUS_SDK_ROOT_DIR} CACHE PATH "Path where to find the Oculus SDK")
	IF(NOT OCULUS_SDK_ROOT_DIR)
		MESSAGE(FATAL_ERROR "Error: Oculus SDK not found.")
	ENDIF(NOT OCULUS_SDK_ROOT_DIR)
  endif(WIN32)

  include_directories(
    BEFORE
    ${OCULUS_SDK_INCLUDE_DIRS}
	${OCULUS_SDK_ROOT_DIR}/LibOVR/Src
   )
endif(OCULUS_SDK)

ADD_SUBDIRECTORY(source)
