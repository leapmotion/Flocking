//
//  Controller.cpp
//  Flocking
//
//  Created by Robert Hodgin on 4/26/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "cinder/app/AppBasic.h"
#include "cinder/gl/Gl.h"
#include "cinder/Rand.h"
#include "Controller.h"

using namespace ci;
using namespace std;

const static float SCALE = 0.4f;

Controller::Controller( int maxTips )
{
	mMaxTips	= maxTips;
	
	// TIME
	mTime			= (float)app::getElapsedSeconds();
	mTimeElapsed	= 0.0f;
	mTimeMulti		= 45.0f;
	mTimer			= 0.0f;
	mTick			= false;
	
//	mNumPredators	= predatorFboDim * predatorFboDim;
//	for( int i=0; i<mNumPredators; i++ ){
//		mPredators.push_back( Predator( Vec3f::zero() ) );
//	}
}

void Controller::updatePredatorBodies( gl::Fbo *fbo )
{
	// BOTH THESE METHODS ARE TOO SLOW.
	// IS THERE NO WAY TO READ OUT THE CONTENTS OF A TINY FBO TEXTURE
	// WITHOUT THE FRAMERATE DROPPING FROM 60 TO 30?
	
//	gl::disableDepthRead();
//	gl::disableDepthWrite();
//	gl::disableAlphaBlending();
//	
//	int index = 0;
//	Surface32f predatorSurface( fbo->getTexture() );
//	Surface32f::Iter it = predatorSurface.getIter();
//	while( it.line() ){
//		while( it.pixel() ){
//			mPredators[index].update( Vec3f( it.r(), it.g(), it.b() ) );
//			index ++;
//		}
//	}
	
//	int index = 0;
//	GLfloat pixel[4];
//	int fboDim = fbo->getWidth();
//	fbo->bindFramebuffer();
//	for( int y=0; y<fboDim; y++ ){
//		for( int x=0; x<fboDim; x++ ){
//			glReadPixels( x, y, 1, 1, GL_RGB, GL_FLOAT, (void *)pixel );
//			mPredators[index].update( Vec3f( pixel[0], pixel[1], pixel[2] ) );
//			index ++;
//		}
//	}
//	fbo->unbindFramebuffer();
}

void Controller::updateLeap( const Leap::HandList &handList )
{
	map<uint32_t,bool>	foundIds;
	map<uint32_t,Vec3f> newTips, movedTips;
	
	// mark all existing touches as not found for now
	for( map<uint32_t,Vec3f>::const_iterator it = mActiveLeapPs.begin(); it != mActiveLeapPs.end(); ++it ){
		foundIds[it->first] = false;
	}
	
	const int numHands = handList.count();
	if (numHands >= 1) {
		
		for( int h=0; h<numHands; h++ ){
			const Leap::Hand& hand = handList[h];
			
			// Check if the hand has any fingers
			const Leap::FingerList &fingers = hand.fingers();
			const int numFingers			= fingers.count();
			if (numFingers >= 1) {
				for( int i = 0; i < numFingers; ++i ){
					
					const Leap::Finger& finger	= fingers[i];
					uint32_t id					= finger.id();
					const Leap::Vector& tipPos	= finger.tipPosition();
					Vec3f pos = Vec3f( tipPos.x, tipPos.y - 200.0f, tipPos.z + 100.0f ) * SCALE;
//					const Leap::Vector *vel = finger.velocity();
					
					if( mActiveLeapPs.find( id ) == mActiveLeapPs.end() ){
						newTips[id]		= pos;
					} else {
						movedTips[id]	= pos;
					}
					
					foundIds[id] = true;
				}
			}
		}
	}
	
	// anybody we didn't find must have ended
	for( map<uint32_t,bool>::const_iterator foundIt = foundIds.begin(); foundIt != foundIds.end(); ++foundIt ){
		if( ! foundIt->second ){
			uint32_t id = foundIt->first;
			mActiveTips[id].startDying();
			mDyingTips.push_back( mActiveTips[id] );
			mActiveTips.erase( id );
		}
	}
	mActiveLeapPs.clear();
	
	// mark any touches new touches
	if( ! newTips.empty() ) {
		for( map<uint32_t,Vec3f>::const_iterator newIt = newTips.begin(); newIt != newTips.end(); ++newIt ){
			uint32_t id		= newIt->first;
			Vec3f pos		= newIt->second;
			
		//	std::cout << "NEW TIP AT " << pos << std::endl;
			mActiveLeapPs[id] = pos;
			mActiveTips.insert( make_pair( id, FingerTip( pos ) ) );
		}
	}
	
	// move anybody who moved
	if( ! movedTips.empty() ) {
		for( map<uint32_t,Vec3f>::const_iterator moveIt = movedTips.begin(); moveIt != movedTips.end(); ++moveIt ){
			uint32_t id		= moveIt->first;
			Vec3f pos		= moveIt->second;
			
			//			Vec3f prevPt	= mActiveLeapPs[id];
			mActiveLeapPs[id] = pos;
			mActiveTips[id].addPoint( pos );
		}
	}
	
	// PARTICLES
//	int numParticles = 10;
//	for( map<uint32_t,FingerTip>::const_iterator activeIt = mActiveTips.begin(); activeIt != mActiveTips.end(); ++activeIt ) {
//		FingerTip tip = activeIt->second;
//		for( int a=0; a<numParticles; a++ ){
//			mParticles.push_back( Particle( tip.mPs[tip.mPs.size()-1], tip.mVs[tip.mVs.size()-1] + Rand::randVec3f() * 0.3f ) );
//		}
//	}
//	
//	for( vector<Particle>::iterator it = mParticles.begin(); it != mParticles.end(); ){
//		if( it->mIsDead ){
//			it = mParticles.erase( it );
//		} else {
//			it->update();
//			it++;
//		}
//	}
	
	mNumTips = 0;
	for( map<uint32_t,FingerTip>::iterator activeIt = mActiveTips.begin(); activeIt != mActiveTips.end(); ++activeIt ) {
		activeIt->second.update();
		mNumTips ++;
	}
	
//	std::cout << "NUM TIPS = " << mNumTips << std::endl;
}

void Controller::updateTime()
{
	float prevTime	= mTime;
	mTime			= (float)app::getElapsedSeconds();
	float dt		= mTime - prevTime;
	mTimeAdjusted	= dt * mTimeMulti;
	mTimeElapsed	+= mTimeAdjusted;
	
	mTimer += mTimeAdjusted;
	mTick = false;
	if( mTimer > 1.0f ){
		mTick = true;
		mTimer = 0.0f;
	}
}

void Controller::update()
{
	updateTime();
	
	// ADD GLOWS AND NEBULA EFFECTS
	if( mTick ){
		for( map<uint32_t,FingerTip>::iterator activeIt = mActiveTips.begin(); activeIt != mActiveTips.end(); ++activeIt ){
			// ADD GLOWS
			int numGlowsToSpawn = 5;
			addGlows( &activeIt->second, numGlowsToSpawn );
			
			// ADD NEBULAS
			int numNebulasToSpawn = 1;
			addNebulas( &activeIt->second, numNebulasToSpawn );
		}
		
		// ADD BUBBLES
		for( int i=0; i<2; i++ ){
			Vec3f pos = Vec3f( Rand::randFloat( -500.0f, 500.0f ), Rand::randFloat( -500.0f, 500.0f ), Rand::randFloat( -250.0f, 50.0f ) );
			addBubbles( pos );
		}
	}
	
//	// SORT LANTERNS
//	sort( mLanterns.begin(), mLanterns.end(), depthSortFunc );
	
	// GLOWS
	for( vector<Glow>::iterator it = mGlows.begin(); it != mGlows.end(); ){
		if( it->mIsDead ){
			it = mGlows.erase( it );
		} else {
			it->update( mTimeAdjusted );
			++ it;
		}
	}
	
	// NEBULAS
	for( vector<Nebula>::iterator it = mNebulas.begin(); it != mNebulas.end(); ){
		if( it->mIsDead ){
			it = mNebulas.erase( it );
		} else {
			it->update( mTimeAdjusted );
			++ it;
		}
	}
	
	// BUBBLES
	for( vector<Bubble>::iterator it = mBubbles.begin(); it != mBubbles.end(); ){
		if( it->mIsDead ){
			it = mBubbles.erase( it );
		} else {
			it->update( mTimeAdjusted );
			++ it;
		}
	}
}

void Controller::drawLanterns( gl::GlslProg *shader )
{
	for( map<uint32_t,FingerTip>::iterator activeIt = mActiveTips.begin(); activeIt != mActiveTips.end(); ++activeIt ) {
		shader->uniform( "color", activeIt->second.mColor );
		activeIt->second.draw();
	}
}

void Controller::drawLanternGlows( const Vec3f &right, const Vec3f &up )
{
	for( map<uint32_t,FingerTip>::iterator activeIt = mActiveTips.begin(); activeIt != mActiveTips.end(); ++activeIt ) {
		FingerTip tip = activeIt->second;
		float radius = tip.mRadius * 10.0f;// * it->mVisiblePer * 10.0f;
		gl::color( Color( tip.mColor ) );
		gl::drawBillboard( tip.mPos, Vec2f( radius, radius ), 0.0f, right, up );
		gl::color( Color::white() );
		gl::drawBillboard( tip.mPos, Vec2f( radius, radius ) * 0.35f, 0.0f, right, up );
	}
}

void Controller::drawGlows( gl::GlslProg *shader, const Vec3f &right, const Vec3f &up )
{
	for( vector<Glow>::iterator it = mGlows.begin(); it != mGlows.end(); ++it ){
		shader->uniform( "alpha", it->mAgePer );
		shader->uniform( "color", it->mColor );
		it->draw( right, up );
	}
}

void Controller::drawNebulas( gl::GlslProg *shader, const Vec3f &right, const Vec3f &up )
{
	for( vector<Nebula>::iterator it = mNebulas.begin(); it != mNebulas.end(); ++it ){
		shader->uniform( "alpha", it->mAgePer );
		shader->uniform( "color", it->mColor );
		it->draw( right, up );
	}
}

void Controller::drawBubbles( gl::GlslProg *shader, const Vec3f &right, const Vec3f &up )
{
	for( vector<Bubble>::iterator it = mBubbles.begin(); it != mBubbles.end(); ++it ){
		shader->uniform( "alpha", sinf( it->mAgePer * (float)M_PI ) );
		shader->uniform( "color", it->mColor );
		it->draw( right, up );
	}
}

void Controller::addGlows( FingerTip *tip, int amt )
{
	for( int i=0; i<amt; i++ ){
		float radius	= Rand::randFloat( 1.0f, 2.0f );
		Vec3f dir		= Rand::randVec3f();
		Vec3f pos		= tip->mPos + dir * ( tip->mRadius - radius );
		dir.xz() *= -0.25f;
		Vec3f vel		= dir * Rand::randFloat( 0.3f, 0.5f );
		float lifespan	= Rand::randFloat( 50.0f, 100.0f );
		Color col		= tip->mColor;
		
		mGlows.push_back( Glow( pos, vel * 0.5f, radius, col, lifespan ) );
		
		if( Rand::randFloat() < 0.01f )
			mGlows.push_back( Glow( pos, Vec3f( vel.x, 1.5f, vel.z ) * 0.5f, radius * 0.5f, col, lifespan * 0.5f ) );
	}
}

void Controller::addNebulas( FingerTip *tip, int amt )
{
	for( int i=0; i<amt; i++ ){
		float radius		= Rand::randFloat( 3.0f, 5.0f );
		Vec3f dir			= Rand::randVec3f();
		Vec3f pos			= tip->mPos + dir * ( tip->mRadius - radius );
		dir.xz() *= -0.25f;
		Vec3f vel			= dir * Rand::randFloat( 0.05f, 0.1f );
		float lifespan		= Rand::randFloat( 50.0f, 120.0f );
		Color col			= tip->mColor;
		
		mNebulas.push_back( Nebula( pos, vel, radius, col, lifespan ) );
		
		if( Rand::randFloat() < 0.01f )
			mNebulas.push_back( Nebula( pos, Vec3f( vel.x, 1.5f, vel.z ), radius * 0.5f, col, lifespan * 0.5f ) );
	}
}

void Controller::addBubbles( const Vec3f &pos )
{
	float radius		= Rand::randFloat( 3.0f, 5.0f );
	Vec3f vel			= Vec3f( Rand::randFloat( -0.5, 0.5f ), Rand::randFloat( 0.0f, 0.5f ), Rand::randFloat( 1.0f, 2.0f ) );
	float lifespan		= Rand::randFloat( 150.0f, 250.0f );
	Color col			= Color( 0.0f, 0.1f, 0.1f );
	
	mBubbles.push_back( Bubble( pos, vel, radius * 10.0f, col, lifespan ) );
}


//bool depthSortFunc( Lantern a, Lantern b ){
//	return a.mPos.z > b.mPos.z;
//}
