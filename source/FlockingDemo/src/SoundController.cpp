//
//  SoundController.cpp
//  Collider
//
//  Created by Robert Hodgin on 2/6/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <boost/foreach.hpp>
#include "cinder/app/AppBasic.h"
#include "SoundController.h"

using namespace ci;
using std::vector;
using std::list;
using std::sort;

#define NUM_TRACKS 2

SoundController::SoundController()
{
}

SoundController::~SoundController()
{
}

#if defined( CINDER_MAC )
void SoundController::init( std::vector<fs::path> loops, std::vector<fs::path> effects )
{
	// AUDIO SETUP
	mVolume				= 1.0f;
	mFmodEarPos			= toFmodVec( Vec3f( 0.0f, 0.0f, -1.0f ) );
	mFmodEarVel			= toFmodVec( Vec3f( 0.0f, 0.0f, 0.0f ) );
	mFmodEarUp			= toFmodVec( Vec3f( 0.0f, 1.0f, 0.0f ) );
	mFmodEarForward		= toFmodVec( Vec3f( 0.0f, 0.0f, -1.0f ) );
	
	for( int i=0; i<loops.size(); i++ ){
		Fmodex3DSoundPlayer *player = new Fmodex3DSoundPlayer();
		player->loadSound( loops[i].string(), false );
		player->setVolume( 1.0f );
		player->setLoop( true );
		player->setMultiPlay( true );
		mFmodPlayers.push_back( player );
	}
	
	mTimeSinceLastTink = 0.0f;
	
	mFmodSys = Fmodex3DSoundPlayer::getSystem();	
	mFmodSys->createSound( effects[0].c_str(), FMOD_3D, 0, &mEffect1 );
//	mFmodSys->createSound( effects[1].c_str(), FMOD_3D, 0, &mEffect2 );
	mFmodSys->createSound( loops[0].c_str(), FMOD_3D, 0, &mLoop1 );
//	mFmodSys->createSound( loops[1].c_str(), FMOD_3D, 0, &mLoop2 );
	
	mEffect1Freq = 22050.0f;
	mEffect2Freq = 22050.0f;
	
	mSpeed = 1.0f;
}

void SoundController::setListener( const CameraPersp &cam, const Vec3f &vel )
{
	mFmodEarPos		= toFmodVec( cam.getCenterOfInterestPoint() );// + Vec3f( 0.0f, 0.0f, 7.0f ) );
	mFmodEarVel		= toFmodVec( vel );
	mFmodEarUp		= toFmodVec( cam.getWorldUp() );
	mFmodEarForward	= toFmodVec(-cam.getViewDirection() );
}

void SoundController::update( float dt )
{
	mFmodSys->set3DListenerAttributes( 0, &mFmodEarPos, &mFmodEarVel, &mFmodEarForward, &mFmodEarUp );
		
	for( int i=0; i<mFmodPlayers.size(); i++ ){
		if( !mFmodPlayers[i]->getIsPlaying() )
			mFmodPlayers[i]->play();
		
		mFmodPlayers[i]->setSpeed( dt );
		mFmodPlayers[i]->setVolume( 1.0f );
		
		float freq = 20000 * dt;
		mFmodPlayers[i]->channel->setFrequency( freq );
	}
	
	mEffect1Channel->setFrequency( mEffect1Freq * dt );
	mEffect2Channel->setFrequency( mEffect2Freq * dt );
}

FMOD_VECTOR SoundController::toFmodVec( Vec3f v )
{
	FMOD_VECTOR fV;
	fV.x = v.x;
	fV.y = v.y;
	fV.z = v.z;
	
	return fV;
}

void SoundController::playEffect1( const Vec3f &v )
{
	FMOD_VECTOR fPos = toFmodVec( v );
	FMOD_VECTOR fVel = toFmodVec( Vec3f::zero() );
	mEffect1Freq		 = Rand::randInt( 20000, 40000 );//( 110 - math<float>::min( p->mImpulse, 100 ) ) * 1000;

	mFmodSys->playSound( FMOD_CHANNEL_REUSE, mEffect1, false, &mEffect1Channel );
	mEffect1Channel->set3DAttributes( &fPos, &fVel );
	mEffect1Channel->setFrequency( mEffect1Freq );
}

//void SoundController::playEffect2( const Vec3f &v )
//{
//	FMOD_VECTOR fPos = toFmodVec( v );
//	FMOD_VECTOR fVel = toFmodVec( Vec3f::zero() );
//	mEffect2Freq		 = Rand::randInt( 20000, 40000 );//( 110 - math<float>::min( p->mImpulse, 100 ) ) * 1000;
//	
//	mFmodSys->playSound( FMOD_CHANNEL_REUSE, mEffect2, false, &mEffect2Channel );
//	mEffect2Channel->set3DAttributes( &fPos, &fVel );
//	mEffect2Channel->setFrequency( mEffect2Freq );
//}



#endif
