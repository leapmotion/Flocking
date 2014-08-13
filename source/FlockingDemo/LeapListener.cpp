//
//  LeapListener.cpp
//  LeapTest
//
//  Created by Robert Hodgin on 10/22/12.
//
//

#include "cinder/app/AppBasic.h"
#include "cinder/app/App.h"
#include "LeapListener.h"

using namespace ci;

LeapListener::LeapListener( std::mutex *mutex )
: Leap::Listener(), mMutex( mutex )
{
	cinder::app::console() << "LeapListener" << std::endl;
}

void LeapListener::onInit(const Leap::Controller& controller) {
	cinder::app::console() << "Initialized" << std::endl;
}

void LeapListener::onConnect(const Leap::Controller& controller) {
	cinder::app::console() << "Connected" << std::endl;
}

void LeapListener::onDisconnect(const Leap::Controller& controller) {
	cinder::app::console() << "Disconnected" << std::endl;
}

void LeapListener::onFrame(const Leap::Controller& controller) {
	std::lock_guard<std::mutex> lock(*mMutex);
	cinder::app::console() << "." << std::endl;
	const Leap::Frame frame = controller.frame();
}