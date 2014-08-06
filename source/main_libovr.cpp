/**
This is an experimental program that used to test initialization of Oculus Rifft DK2 
*/

#include "Windows.h"
#include "OVR.h"
#pragma comment(lib,"Ws2_32.lib")

// Choose whether the SDK performs rendering/distortion, or the application. 
#define          SDK_RENDER 1 

//Should be true for correct timing.  Use false for debug only
const bool       FullScreen = false; 

//Structures for the application
ovrHmd             HMD;
ovrEyeRenderDesc   EyeRenderDesc[2];
ovrRecti           EyeRenderViewport[2];

// We can use our own ?
//RenderDevice*      pRender = 0;
//Texture*           pRendertargetTexture = 0;
//Scene*             pRoomScene = 0;


int main (int argc, char **argv)
{
	// Initializes LibOVR, and the Rift
	ovr_Initialize();
	ovrHmd hmd = ovrHmd_Create(0);
	ovrHmdDesc hmdDesc;

	if (!hmd){
		MessageBoxA(NULL, "Oculus Rift not detected.", "", MB_OK);
		return(1);
	}
	else{
		MessageBoxA(NULL, "Oculus Rift is detected.", "", MB_OK);
	}

	//Setup Window and Graphics - use window frame if relying on Oculus driver
	const int backBufferMultisample = 1;
	bool UseAppWindowFrame = (HMD->HmdCaps & ovrHmdCap_ExtendDesktop) ? false : true;
	
	ovrBool ovrHmd_StartSensor(ovrHmd hmd,
		unsigned int supportedSensorCaps,
		unsigned int requiredSensorCaps);

	ovrHmd_Destroy(hmd);
	ovr_Shutdown();
	
	return 0;
}