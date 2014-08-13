//
//  FingerTip.cpp
//  LeapPrototype
//
//  Created by Robert Hodgin on 11/1/12.
//
//

#include "FingerTip.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

using namespace ci;

FingerTip::FingerTip()
{
}

FingerTip::FingerTip( const Vec3f &initPt )
	: mTimeOfDeath( -1.0f ), mPos( initPt )
{
	mIsDying = false;
	
	mAggression = 1.0f;	// negative = aggressive
	mRadius		= 0.0f;
	mRadiusDest	= 2.0f;
	
	mHue		= 1.0f;
	mColor		= Color( 0.1f, 0.6f, 0.6f );
}

void FingerTip::addPoint( const ci::Vec3f &pos )
{
	Vec3f prevPos = mPos;
	mPos = pos;
	mVel = ( mPos - prevPos ) * 0.5f;
	
	if( mVel.length() > 2.0f ){								// if aggressive movement,
		mAggression -= ( mAggression - (-5.0f) ) * 0.1f;		// increase aggression 
	} else {												// else
		mAggression -= ( mAggression - ( 1.0f) ) * 0.01f;		// decrease slowly
	}
	
	float aggPer = 1.0f - ( mAggression + 5.0f )/6.0f;
	mHue = aggPer * 0.3f + 0.05f;
	
	mColor		= Color( CM_HSV, mHue, 0.9f, 1.0f );
}

void FingerTip::update()
{	
	mRadius -= ( mRadius - mRadiusDest ) * 0.2f;
}

void FingerTip::draw() const
{
	gl::drawSphere( mPos, mRadius * 0.5f, 32 );
}

void FingerTip::startDying()
{
	mIsDying = true;
	mTimeOfDeath = (float)app::getElapsedSeconds() + 2.0f;
}

bool FingerTip::isDead()
{
	return app::getElapsedSeconds() > mTimeOfDeath;
}