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

set(Cinder_LIBRARY_DIR ${Cinder_ROOT_DIR}/lib/msw/x86)

find_library(Cinder_LIBRARY_RELEASE "cinder-v120.lib" HINTS "${Cinder_LIBRARY_DIR}")
find_library(Cinder_LIBRARY_DEBUG "cinder-v120_d.lib" HINTS "${Cinder_LIBRARY_DIR}")

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
)

set_source_files_properties( Resources.rc PROPERTIES LANGUAGE RC )

add_executable(${TARGET_TARGETNAME} WIN32 ${TARGET_SRC})

add_definitions(-DUNICODE)

### Set compiler flags ####
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT") # Cinder's HACK
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MT") # Cinder's HACK
STRING (REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}") # Remove RTC so we can use Debug build

#### Set Linker flags ####
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LIBPATH:${Cinder_LIBRARY_DIR}") # Cinder's HACK: Needed to find the boost libraries
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:LIBCMT") #  Cinder's HACK: This is in platform too so it's probably good

#find_package(Components)
# The target_link_components function finds and links exactly the minimal set of libraries
# needed by the specified components.
#target_link_components(${TARGET_TARGETNAME} PUBLIC Application GLController GLShaderLoader GLTexture2Loader )
#target_link_components(${TARGET_TARGETNAME} PUBLIC Application GLController GLShaderLoader OculusVR Primitives SDLController)

target_package(${TARGET_TARGETNAME} Leap LINK_TYPE PUBLIC)
target_package(${TARGET_TARGETNAME} OculusSDK LINK_TYPE PUBLIC)

target_package(${TARGET_TARGETNAME} Glew LINK_TYPE PUBLIC)
target_package(${TARGET_TARGETNAME} SDL LINK_TYPE PUBLIC)
target_package(${TARGET_TARGETNAME} Eigen LINK_TYPE PUBLIC)

target_link_libraries(${TARGET_TARGETNAME} PUBLIC winmm.lib) # Oculus's HACK (not ours)
target_link_libraries(${TARGET_TARGETNAME} PUBLIC	Ws2_32.lib) # Oculus's HACK (not ours)

target_link_libraries(${TARGET_TARGETNAME} PUBLIC optimized ${Cinder_LIBRARY_RELEASE}) # OUR hack
target_link_libraries(${TARGET_TARGETNAME} PUBLIC debug ${Cinder_LIBRARY_DEBUG}) # OUR hack
