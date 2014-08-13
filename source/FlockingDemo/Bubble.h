
#pragma once

#include "cinder/Vector.h"
#include "cinder/Color.h"

class Bubble {
public:
	
	Bubble();
	Bubble( const ci::Vec3f &pos, const ci::Vec3f &vel, float radius, ci::Color c, float lifespan );
	void update( float timeDelta );
	void draw( const ci::Vec3f &right, const ci::Vec3f &up );
	
	ci::Vec3f	mPos;
	ci::Vec3f	mVel;
	float		mRot;
	float		mRadius, mRadiusDest;
	float		mAge, mAgePer, mLifespan;
	ci::Color	mColor;
	bool		mIsDead;
};