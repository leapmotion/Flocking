set(TARGET_TARGETNAME SpaceEX)

SET(TARGET_SRC
	ExampleLayer.h
	ExampleLayer.cpp
	SpaceEXApplication.h
	SpaceEXApplication.cpp
	main.cpp
   )
   
ADD_EXECUTABLE(${TARGET_TARGETNAME} ${TARGET_SRC})

target_imported_libraries(${TARGET_TARGETNAME} PUBLIC SDL::SDL)
target_link_libraries(${TARGET_TARGETNAME} PUBLIC Components::Components)

