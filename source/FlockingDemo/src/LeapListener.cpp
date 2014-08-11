//
//  LeapListener.cpp
//  LeapTest
//
//  Created by Robert Hodgin on 10/22/12.
//
//

#include "cinder/app/AppBasic.h"
#include "LeapListener.h"

using namespace ci;

LeapListener::LeapListener( std::mutex *mutex )
: Leap::Listener(), mMutex( mutex )
{
}

void LeapListener::onInit(const Leap::Controller& controller) {
	std::cout << "Initialized" << std::endl;
}

void LeapListener::onConnect(const Leap::Controller& controller) {
	std::cout << "Connected" << std::endl;
}

void LeapListener::onDisconnect(const Leap::Controller& controller) {
	std::cout << "Disconnected" << std::endl;
}

void LeapListener::onFrame(const Leap::Controller& controller) {
	std::lock_guard<std::mutex> lock( *mMutex );
	const Leap::Frame frame = controller.frame();
}