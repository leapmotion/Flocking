//
//  Controller.h
//  Flocking
//
//  Created by Robert Hodgin on 4/26/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#pragma once
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "Room.h"
#include "Glow.h"
#include "Nebula.h"
#include "Bubble.h"
#include "Leap.h"
#include "FingerTip.h"

#include <vector>
#include <map>
#include <list>

class Controller {
public:
	Controller( int maxTips );
	void update();
	void updateTime();
	void updateLeap( const Leap::HandList &handList );
	void updatePredatorBodies( ci::gl::Fbo *fbo );
	void drawLanterns( ci::gl::GlslProg *shader );
	void drawLanternGlows( const ci::Vec3f &right, const ci::Vec3f &up );
	void drawGlows( ci::gl::GlslProg *shader, const ci::Vec3f &right, const ci::Vec3f &up );
	void drawNebulas( ci::gl::GlslProg *shader, const ci::Vec3f &right, const ci::Vec3f &up );
	void drawBubbles( ci::gl::GlslProg *shader, const ci::Vec3f &right, const ci::Vec3f &up );
	void addGlows( FingerTip *tip, int amt );
	void addNebulas( FingerTip *tip, int amt );
	void addBubbles( const ci::Vec3f &pos );
  ci::Vec3f transformPos(const Leap::Vector& tipPos) const;

	std::map<uint32_t,FingerTip>	mActiveTips;
	std::list<FingerTip>			mDyingTips;
	
	std::map<uint32_t,ci::Vec3f>	mActiveLeapPs;
	
	// TIME
	float			mTime;				// Time elapsed in real world seconds
	float			mTimeElapsed;		// Time elapsed in simulation seconds
	float			mTimeMulti;			// Speed at which time passes
	float			mTimeAdjusted;		// Amount of time passed between last frame and current frame
	float			mTimer;				// A resetting counter for determining if a Tick has occured
	bool			mTick;				// Tick (aka step) for triggering discrete events

	int						mNumTips;
	int						mMaxTips;
	std::vector<Glow>		mGlows;
	std::vector<Nebula>		mNebulas;
	std::vector<Bubble>		mBubbles;

  ci::Matrix44f m_invRot;
};

//bool depthSortFunc( Lantern a, Lantern b );


