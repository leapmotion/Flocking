#ifdef OCULUS_SDK
#define GLEW_STATIC
#include "GL/glew.h"
#include "OVR.h"

#define OVR_OS_WIN32

#include "GLController.h"
#include "OVR_CAPI_GL.h"

#include "Kernel/OVR_Math.h"
#include "SDLController.h"

using namespace OVR;

#else
#include "SpaceEXApplication.h"
#endif

int main (int argc, char **argv)
{
   #ifdef OCULUS_SDK
	SDL_Init(SDL_INIT_VIDEO);

	int x = SDL_WINDOWPOS_CENTERED;
	int y = SDL_WINDOWPOS_CENTERED;
	Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

	bool debug = false;

	ovr_Initialize();
	Sizei WindowSize;
	ovrHmd hmd = ovrHmd_Create(0);

	if (!hmd)
	{
		// If we didn't detect an Hmd, create a simulated one for debugging.
		hmd = ovrHmd_CreateDebug(ovrHmd_DK1);
		debug = true;
		if (!hmd)
		{   // Failed Hmd creation.
			return 1;
		}
	}

	if (debug == false)
	{
		x = hmd->WindowsPos.x;
		y = hmd->WindowsPos.y;
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	
	int w = hmd->Resolution.w;
	int h = hmd->Resolution.h;

	// This block takes care of the SDL related 
	// initialization and window creation
	SDLController m_SDLController;
	SDLControllerParams params;
	params.windowHeight = h;
	params.windowWidth = w;
	params.windowPosX = x;
	params.windowPosY - y;
	params.transparentWindow = true;
	params.alwaysOnTop = false;
	params.fullscreen = false;
	params.windowTitle = "Oculus Rift SDL2 OpenGL Demo";
	params.antialias = true;
	params.vsync = true;

	m_SDLController.Initialize(params);

  #else
	StubApplication app;
	// StubApplication::Initialize is what sets everything up,
	// and StubApplication::Shutdown is what tears it down.
	// This call to RunApplication is what drives the application (it
	// contains e.g. the game loop, with event handling, etc).
	RunApplication(app);
  #endif
    return 0;
}

