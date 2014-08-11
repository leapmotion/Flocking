

#include "Bubble.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

using namespace ci;

Bubble::Bubble()
{
}

Bubble::Bubble( const Vec3f &pos, const Vec3f &vel, float radius, Color c, float lifespan )
{
	mPos			= pos;
	mVel			= vel;
	mRadiusDest		= radius * 2.0f;
	mRadius			= mRadiusDest;
	mColor			= c;
	mAge			= 0.0f;
	mLifespan		= lifespan;
	mIsDead			= false;
	mRot			= Rand::randFloat( 360.0f );
	
//	if( Rand::randFloat() < 0.01f ){
//		mRadius *= 2.0f;
//		mLifespan *= 1.3f;
//	}
}

void Bubble::update( float dt )
{
//	mRadius -= ( mRadius - mRadiusDest ) * 0.1f * dt;
	mPos	+= mVel * dt;
	mVel	-= mVel * 0.01f * dt;
	mVel	+= Vec3f( 0.0f, 0.002f, 0.0f ) * dt;
	mAge	+= dt;
	mAgePer  = 1.0f - mAge/mLifespan;
	
	if( mAge > mLifespan ){
		mIsDead = true;
	}
}

void Bubble::draw( const Vec3f &right, const Vec3f &up )
{
	gl::drawBillboard( mPos, ci::Vec2f( mRadius, mRadius ), mRot, right, up );
}
