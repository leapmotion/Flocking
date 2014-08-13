cmake_minimum_required(VERSION 3.0) # I had problems if I only specified "VERSION 3".

project(OculusAPPs)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake-modules")
set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "" FORCE)

include(ListSubdirectories)
include(TargetImportedLibraries)

set(TARGET_TARGETNAME FlockingDemo)

########### Find Cinder ###########
find_path(Cinder_ROOT_DIR
			NAMES include/cinder/Cinder.h
			PATH_SUFFIXES cinder_${Cinder_FIND_VERSION}_${_compiler_SUFFIX}
						cinder_${Cinder_FIND_VERSION}
						cinder)


find_path(Cinder_INCLUDE_DIR 
			NAMES "cinder/Cinder.h"
			HINTS "${Cinder_ROOT_DIR}"
			PATH_SUFFIXES "include")

set(Cinder_LIBRARY_DIR ${Cinder_ROOT_DIR}/lib/msw/x86)

find_library(Cinder_LIBRARY_RELEASE "cinder-v120.lib" HINTS "${Cinder_LIBRARY_DIR}")
find_library(Cinder_LIBRARY_DEBUG "cinder-v120_d.lib" HINTS "${Cinder_LIBRARY_DIR}")

include_directories(
    ${Cinder_ROOT_DIR}/include
	${Cinder_ROOT_DIR}/boost
   )

########### Find Oculus ###########
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
)

set_source_files_properties( Resources.rc PROPERTIES LANGUAGE RC )

add_executable(${TARGET_TARGETNAME} ${TARGET_SRC})

add_definitions(-DUNICODE)

### Set compiler flags ####
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")

#### Set Linker flags ####
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}  /DEBUG /MACHINE:X86 /OPT:REF /SAFESEH /INCREMENTAL:NO")  
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}  /SUBSYSTEM:WINDOWS /DYNAMICBASE:NO")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}  /MAP /ERRORREPORT:PROMPT /NOLOGO")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}  /LIBPATH:${Cinder_LIBRARY_DIR}")

target_package(${TARGET_TARGETNAME} Components)
target_package(${TARGET_TARGETNAME} Leap)

target_link_libraries(${TARGET_TARGETNAME} winmm.lib
										   Ws2_32.lib
                                           ${OCULUS_SDK_LIBRARY} 
										   ${Cinder_LIBRARY_RELEASE} 
										   )
