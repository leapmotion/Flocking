//
//  SoundController.h
//  Collider
//
//  Created by Robert Hodgin on 2/6/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#pragma once
//#include "HodginUtility.h"
#include "cinder/Rand.h"
#include "cinder/Camera.h"
#include <vector>

#if defined( CINDER_MAC )
#include "Fmodex3DSoundPlayer.h"
#endif

class SoundController {
public:
	SoundController();
	~SoundController();
#if defined( CINDER_MAC )	
	void		init( std::vector<ci::fs::path> loops, std::vector<ci::fs::path> effects );
	void		update( float timeMulti );
	void		setListener( const ci::CameraPersp &cam, const ci::Vec3f &vel );
	void		playEffect1( const ci::Vec3f &v );
	void		playEffect2( const ci::Vec3f &v );
//	void		playTink( Particle *p );
	
	float				mVolume;
	float				mTimeSinceLastTink;
	
	FMOD_VECTOR toFmodVec( ci::Vec3f v );
	FMOD::System*		mFmodSys;
	//	std::vector<Fmodex3DSoundPlayer> mFmodPlayers;
	std::vector<Fmodex3DSoundPlayer*> mFmodPlayers;
	FMOD_VECTOR			mFmodEarPos;
	FMOD_VECTOR			mFmodEarVel;
	FMOD_VECTOR			mFmodEarUp;
	FMOD_VECTOR			mFmodEarForward;
	
	FMOD::Sound			*mEffect1;
	FMOD::Channel		*mEffect1Channel;
	int					mEffect1Freq;
	
	FMOD::Sound			*mEffect2;
	FMOD::Channel		*mEffect2Channel;
	int					mEffect2Freq;
	
	FMOD::Sound			*mLoop1;
	FMOD::Channel		*mLoop1Channel;
	
	FMOD::Sound			*mLoop2;
	FMOD::Channel		*mLoop2Channel;
	
//	std::vector<FMOD::Channel*> mPopChannels;
//	
//	FMOD::Sound			*mStarBirth;
//	FMOD::Channel		*mStarBirthChannel;
//	float				mStarBirthFreq;
//	
//	FMOD::Sound			*mNova;
//	FMOD::Channel		*mNovaChannel;
//	float				mNovaFreq;
	
	float				mSpeed;
#endif
};