cmake_minimum_required(VERSION 3.0) # I had problems if I only specified "VERSION 3".

project(SpaceEX)

#SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" ${CMAKE_CURRENT_SOURCE_DIR}/modules)
SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake-modules)

# Show relative paths options
OPTION(CMAKE_USE_RELATIVE_PATHS "If true, cmake will use relative paths in makefiles and projects." ON)
MARK_AS_ADVANCED(CMAKE_USE_RELATIVE_PATHS)

find_path(EXTERNAL_LIBRARY_DIR "glew-1.9.0" HINTS $ENV{EXTERNAL_LIBRARY_DIR} NO_DEFAULT_PATH)

if (EXTERNAL_LIBRARY_DIR STREQUAL "EXTERNAL_LIBRARY_DIR-NOTFOUND")  
  message(STATUS "External Library Directory not found, please specify a folder to look for external libraries")  
  return()
endif()

# Please keep these in alphabetical order.
find_package(OculusSDK REQUIRED)
find_package(Components REQUIRED)

get_target_property(Components_LIBRARIES Components IMPORTED_LOCATION_DEBUG)
message("Components_LIBRARIES = ${Components_LIBRARIES}")

find_package(Eigen 3.1.2 REQUIRED)
find_package(FbxSdk REQUIRED)
find_package(FreeImage REQUIRED)
find_package(Glew 1.9.0 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SDL 2 REQUIRED)


if(${CMAKE_BUILD_TYPE} MATCHES Release)
    set(FreeImage_LIBRARIES ${FreeImage_LIBRARY_RELEASE})
else()
    set(FreeImage_LIBRARIES ${FreeImage_LIBRARY_DEBUG})
endif()
message("FreeImage_LIBRARIES = ${FreeImage_LIBRARIES}")

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

ADD_SUBDIRECTORY(sources)

