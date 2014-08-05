set(TARGET_TARGETNAME SpaceEX)

SET(TARGET_SRC
	ExampleLayer.h
	ExampleLayer.cpp
	SpaceEXApplication.h
    OculusRoomModel.cpp
	RenderTiny_D3D11_Device.cpp
	Win32_OculusRoomTiny.cpp
	Win32_OculusRoomTiny_Util.cpp
	OSX_OculusRoomTiny2.h
	RenderTiny_D3D11_Device.h
	#main.cpp
   )

ADD_EXECUTABLE(${TARGET_TARGETNAME} ${TARGET_SRC})

add_definitions(-DUNICODE)

TARGET_LINK_LIBRARIES(${TARGET_TARGETNAME} Components)
TARGET_LINK_LIBRARIES(${TARGET_TARGETNAME} ${FreeImage_LIBRARIES})

# Link to libraries needed for Oculus on Windows
IF(WIN32)
  TARGET_LINK_LIBRARIES(${TARGET_TARGETNAME} "${OCULUS_SDK_LIBRARY}" 
											winmm.lib
											ws2_32.lib
											Dbghelp.lib
											libovrd.lib
											dxgi.lib
											dxguid.lib
											d3d10.lib
											d3d11.lib
											d3dcompiler.lib
											kernel32.lib
											user32.lib
											gdi32.lib
											winspool.lib
											comdlg32.lib
											advapi32.lib
											shell32.lib
											ole32.lib
											oleaut32.lib
											uuid.lib
											odbc32.lib
											odbccp32.lib
											)

ENDIF(WIN32)
