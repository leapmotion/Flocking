//
//  FingerTip.h
//  LeapPrototype
//
//  Created by Robert Hodgin on 11/1/12.
//
//

#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/BSpline.h"

class FingerTip {
  public:
	FingerTip();
	FingerTip( const ci::Vec3f &initPt );
	
	void addPoint( const ci::Vec3f &pos );
	void update();
	void draw() const;
	void startDying();
	bool isDead();

	ci::Vec3f				mPos;
	ci::Vec3f				mVel;
	
	float					mHue;
	float					mAggression;
	ci::Color				mColor;
	float					mRadius, mRadiusDest;
	
	float					mTimeOfDeath;
	bool					mIsDying;
};