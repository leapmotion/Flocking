#ifdef OCULUS_SDK
#define GLEW_STATIC
#include "GL/glew.h"
#include "OVR.h"

#define OVR_OS_WIN32

#include "OVR_CAPI_GL.h"
#include "Kernel/OVR_Math.h"
#include "SDL.h"
#include "SDL_syswm.h"

using namespace OVR;
#else
#include "SpaceEXApplication.h"
#endif

int main (int argc, char **argv)
{
  #ifdef OCULUS_SDK
	// Oculus related coe goes here
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

