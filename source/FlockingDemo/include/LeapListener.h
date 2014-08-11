//
//  LeapListener.h
//  LeapTest
//
//  Created by Robert Hodgin on 10/22/12.
//
//

#pragma once
#include "cinder/Thread.h"
#include "Leap.h"
#include <iostream>

class LeapListener : public Leap::Listener {
public:
	LeapListener( std::mutex *mutex );
    virtual void onInit( const Leap::Controller& );
    virtual void onConnect( const Leap::Controller& );
    virtual void onDisconnect( const Leap::Controller& );
    virtual void onFrame( const Leap::Controller& );
	
	std::mutex	*mMutex;
};
