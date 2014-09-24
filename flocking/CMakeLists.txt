cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-module")
include(Common)

project(FlockingDemo)
set(TARGET_TARGETNAME FlockingDemo)

########### Find Cinder ###########
set(Cinder_FIND_VERSION "0.8.6")
find_path(Cinder_ROOT_DIR
			NAMES include/cinder/Cinder.h
			PATH_SUFFIXES cinder_${Cinder_FIND_VERSION}_vc2013
						cinder_${Cinder_FIND_VERSION}
						cinder)

find_path(Cinder_INCLUDE_DIR 
			NAMES "cinder/Cinder.h"
			HINTS "${Cinder_ROOT_DIR}"
			PATH_SUFFIXES "include")

if( SHOW_CONSOLE AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC" )
 set(Cinder_LIBRARY_DIR ${Cinder_ROOT_DIR}/lib/msw/x86)
 find_library(Cinder_LIBRARY_RELEASE "cinder-v120.lib" HINTS "${Cinder_LIBRARY_DIR}")
 find_library(Cinder_LIBRARY_DEBUG "cinder-v120_d.lib" HINTS "${Cinder_LIBRARY_DIR}")
elseif( APPLE )
 find_library(Cinder_LIBRARY_RELEASE "libcinder.a" HINTS "${Cinder_ROOT_DIR}/lib")
 find_library(Cinder_LIBRARY_DEBUG "libcinder_d.a" HINTS "${Cinder_ROOT_DIR}/lib")
endif()

include_directories(
    ${Cinder_ROOT_DIR}/include
    ${Cinder_ROOT_DIR}/boost
)

SET(TARGET_SRC
  Bubble.cpp
  FlockingApp.cpp   
  Nebula.cpp    
  SoundController.cpp
  Controller.cpp  
  Glow.cpp          
  Predator.cpp
  SpringCam.cpp
  FingerTip.cpp   
  LeapListener.cpp  
  Room.cpp
  Bubble.h      
  Glow.h          
  Predator.h   
  SoundController.h
  Controller.h  
  LeapListener.h  
  Resources.h
  Resources.rc
  SpringCam.h
  FingerTip.h   
  Nebula.h        
  Room.h
  Mirror.cpp
  Mirror.h
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(_add_executable_options "MACOSX_BUNDLE")
    if (NOT CMAKE_OSX_ARCHITECTURES)
      set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Mac OS X build architectures" FORCE)
    endif()
    if (NOT CMAKE_OSX_SYSROOT)
      set(CMAKE_OSX_SYSROOT "macosx10.8" CACHE STRING "Mac OS X build environment" FORCE)
    endif()
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wall -Werror -Wno-uninitialized -Wno-unused-private-field -Wno-reorder -Wno-unused-variable”)

  # Locate all recourses
  FILE(GLOB COPY_RESOURCES ${PROJECT_SOURCE_DIR}/resources/*)
  SET_SOURCE_FILES_PROPERTIES(
    "${COPY_RESOURCES}"
    PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources
  )

  add_executable(
    ${TARGET_TARGETNAME} 
    ${_add_executable_options} 
    ${TARGET_SRC} 
    ${COPY_RESOURCES} 
  )

  # Add resources to application bundle target
  SET_TARGET_PROPERTIES(
    ${TARGET_TARGETNAME}
    PROPERTIES
    RESOURCE "${COPY_RESOURCES}"
  )
elseif ( SHOW_CONSOLE AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC" )
  set_source_files_properties( Resources.rc PROPERTIES LANGUAGE RC )
 
  add_executable(${TARGET_TARGETNAME} WIN32 ${TARGET_SRC})
 
  ### Set compiler flags and definitions ####
  
  add_definitions(-DUNICODE)
  
  # Cinder's HACK
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  
  # Cinder's HACK
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MT")

  # Remove RTC so we can use Debug build
  STRING (REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  
  #### Set Linker flags ####

  # Cinder's HACK: Needed to find the boost libraries
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LIBPATH:${Cinder_LIBRARY_DIR}") 

  # Cinder's HACK: This is in platform too so it's probably good
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:LIBCMT")
endif()

find_package(Components)
target_link_components(
  ${TARGET_TARGETNAME} PUBLIC
  Application
  GLController
  GLShaderLoader
  OculusVR
  Primitives
)

target_package(${TARGET_TARGETNAME} Leap LINK_TYPE PUBLIC)
target_package(${TARGET_TARGETNAME} OculusSDK LINK_TYPE PUBLIC)
target_package(${TARGET_TARGETNAME} Glew LINK_TYPE PUBLIC)
target_package(${TARGET_TARGETNAME} SDL LINK_TYPE PUBLIC)
target_package(${TARGET_TARGETNAME} Eigen LINK_TYPE PUBLIC)

if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  target_link_libraries(
		${TARGET_TARGETNAME} 
		PUBLIC optimized ${Cinder_LIBRARY_RELEASE}
		PUBLIC debug ${Cinder_LIBRARY_DEBUG}
		"-framework Cocoa"
		"-framework OpenGL"
		"-framework Carbon"
		"-framework CoreVideo"
		"-framework QTKit"
		"-framework Accelerate"
		"-framework AudioToolbox"
		"-framework AudioUnit"
		"-framework CoreAudio")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
  target_link_libraries(
		${TARGET_TARGETNAME} PUBLIC winmm.lib      # Oculus's HACK
		PUBLIC Ws2_32.lib  			   # Oculus's HACK
		PUBLIC optimized ${Cinder_LIBRARY_RELEASE} # OUR hack
		PUBLIC debug ${Cinder_LIBRARY_DEBUG}       # OUR hack
		PUBLIC comctl32.lib Dwmapi.lib)
endif()
