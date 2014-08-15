#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Utilities.h"
#include "cinder/ImageIo.h"
#include "cinder/Camera.h"
#include "cinder/Perlin.h"
#include "cinder/Rand.h"
#include "cinder/params/Params.h"
#include "cinder/Thread.h"

#include "Resources.h"
//#include "HodginUtility.h"
#include "SpringCam.h"
#include "Room.h"
#include "Controller.h"

#include "LeapListener.h"
#include "Leap.h"
#include <iostream>

// OVR SDK 
#include "OVR.h"

#define OVR_OS_WIN32

#include "OVR_CAPI_GL.h"
#include "Kernel/OVR_Math.h"
#include "SDL.h"
#include "SDL_syswm.h"

using namespace OVR;

using namespace ci;
using namespace ci::app;
using namespace std;

#define APP_WIDTH		1280
#define APP_HEIGHT		720
#define FBO_DIM			50
#define P_FBO_DIM		5
#define MAX_TIPS		10

class FlockingApp : public AppBasic {
public:
  virtual void		prepareSettings( Settings *settings );
  virtual void		setup();
  void				adjustFboDim( int offset );
  void				initialize();
  void				setFboPositions( gl::Fbo fbo );
  void				setFboVelocities( gl::Fbo fbo );
  void				initVbo();
  virtual void		mouseDown( MouseEvent event );
  virtual void		mouseUp( MouseEvent event );
  virtual void		mouseMove( MouseEvent event );
  virtual void		mouseDrag( MouseEvent event );
  virtual void		keyDown( KeyEvent event );
  virtual void		update();
  void				updateLeap();
  void				drawIntoVelocityFbo();
  void				drawIntoPositionFbo();
  void				drawIntoFingerTipsFbo();
  void				drawGlows();
  void				drawNebulas();
  void				drawBubbles();
  void				drawTitle();
  virtual void		draw();
  void				CalculateHmdValues();
  
  std::mutex		mMutex;
  
  // CAMERA
  SpringCam			mSpringCam;
  
  // TEXTURES
  gl::Texture			mLanternGlowTex;
  gl::Texture			mGlowTex;
  gl::Texture			mNebulaTex;
  gl::Texture			mBubbleTex;
  gl::Texture			mBgTex;
  gl::Texture			mTitleTex;
  
  // SHADERS
  gl::GlslProg		mVelocityShader;
  gl::GlslProg		mPositionShader;
  gl::GlslProg		mLanternShader;
  gl::GlslProg		mRoomShader;
  gl::GlslProg		mShader;
  gl::GlslProg		mGlowShader;
  gl::GlslProg		mNebulaShader;
  
  // LEAP
  LeapListener		*mListener;
  Leap::Controller	*mLeapController;
  
  // CONTROLLER
  Controller			*mController;
  
  // FINGERTIPS FBO (point lights)
  gl::Fbo				mFingerTipsFbo;
  
  // VBOS
  gl::VboMesh			mVboMesh;
  gl::VboMesh			mP_VboMesh;
  
  // POSITION/VELOCITY FBOS
  gl::Fbo::Format		mRgba16Format;
  int					mFboDim;
  ci::Vec2f				mFboSize;
  ci::Area				mFboBounds;
  gl::Fbo				mPositionFbos[2];
  gl::Fbo				mVelocityFbos[2];
  int					mThisFbo, mPrevFbo;
  
  // PERLIN
  Perlin				mPerlin;
  
  // MOUSE
  Vec2f				mMousePos, mMouseDownPos, mMouseOffset;
  bool				mMousePressed;
  
  bool				mInitUpdateCalled;

  // OVR SDK variables 
  ovrHmd hmd;
  ovrSizei resolution;
  ovrSizei w;
  ovrSizei h;

  // target textures
  ovrSizei recommendedTex0Size;
  ovrSizei recommendedTex1Size;

  ovrSizei renderTargetSize;

#if _WIN32
  HWND m_HWND;
#endif

  ovrGLTexture  eyeTexture[2];
  ovrEyeRenderDesc eyeRenderDesc[2];
  ovrRecti eyeRenderViewport[2];

  ci::gl::Fbo m_HMDFbo;

};

void FlockingApp::prepareSettings( Settings *settings )
{
  settings->setWindowSize( APP_WIDTH, APP_HEIGHT );
}

void FlockingApp::setup()
{
  // LEAP
  mListener			= new LeapListener( &mMutex );
  mLeapController		= new Leap::Controller();
  mLeapController->addListener( *mListener );
  
  // CAMERA	
  mSpringCam			= SpringCam( 100.0f, getWindowAspectRatio() );
  
  // POSITION/VELOCITY FBOS
  mRgba16Format.setColorInternalFormat( GL_RGBA16F_ARB );
  mRgba16Format.setMinFilter( GL_NEAREST );
  mRgba16Format.setMagFilter( GL_NEAREST );
  mThisFbo				= 0;
  mPrevFbo				= 1;
  
  // LANTERNS
  mFingerTipsFbo			= gl::Fbo( MAX_TIPS, 2, mRgba16Format );
  
  // TEXTURE FORMAT
  gl::Texture::Format mipFmt;
    mipFmt.enableMipmapping( true );
    mipFmt.setMinFilter( GL_LINEAR_MIPMAP_LINEAR );    
    mipFmt.setMagFilter( GL_LINEAR );
  
  // TEXTURES
  mLanternGlowTex			= gl::Texture( loadImage( loadResource( RES_LANTERNGLOW_PNG ) ) );
  mGlowTex				= gl::Texture( loadImage( loadResource( RES_GLOW_PNG ) ) );
  mNebulaTex				= gl::Texture( loadImage( loadResource( RES_NEBULA_PNG ) ) );
  mBubbleTex				= gl::Texture( loadImage( loadResource( RES_BUBBLE_PNG ) ) );
  mBgTex					= gl::Texture( loadImage( loadResource( RES_BG_PNG ) ) );
  mTitleTex				= gl::Texture( loadImage( loadResource( RES_TITLE_PNG ) ) );
  
  // LOAD SHADERS
  try {
    mVelocityShader		= gl::GlslProg( loadResource( RES_PASSTHRU_VERT ),	loadResource( RES_VELOCITY_FRAG ) );
    mPositionShader		= gl::GlslProg( loadResource( RES_PASSTHRU_VERT ),	loadResource( RES_POSITION_FRAG ) );
    mLanternShader		= gl::GlslProg( loadResource( RES_LANTERN_VERT ),	loadResource( RES_LANTERN_FRAG ) );
    mRoomShader			= gl::GlslProg( loadResource( RES_ROOM_VERT ),		loadResource( RES_ROOM_FRAG ) );
    mShader				= gl::GlslProg( loadResource( RES_VBOPOS_VERT ),	loadResource( RES_VBOPOS_FRAG ) );
    mGlowShader			= gl::GlslProg( loadResource( RES_PASSTHRU_VERT ),	loadResource( RES_GLOW_FRAG ) );
    mNebulaShader		= gl::GlslProg( loadResource( RES_PASSTHRU_VERT ),	loadResource( RES_NEBULA_FRAG ) );
  } catch( gl::GlslProgCompileExc e ) {
    std::cout << e.what() << std::endl;
    quit();
  }

  // CONTROLLER
  mController			= new Controller( MAX_TIPS );
  
  // PERLIN
  mPerlin				= Perlin( 4 );
  
  // MOUSE
  mMousePos			= Vec2f::zero();
  mMouseDownPos		= Vec2f::zero();
  mMouseOffset		= Vec2f::zero();
  mMousePressed		= false;

  mInitUpdateCalled	= false;
  
  initialize();
}

void FlockingApp::initialize()
{
 
  bool debug = false;
  
  ovr_Initialize();

  // ovrHmd handle is actually a pointer to an ovrHmdDesc struct that 
  // contains information about the HMD and its capabilities, and is 
  // used to set up rendering.
  hmd = ovrHmd_Create(0);

  if (!hmd)
  {
	  // If we didn't detect an Hmd, create a simulated one for debugging.
	  hmd = ovrHmd_CreateDebug(ovrHmd_DK2);
	  debug = true;
	  if (!hmd)
	  {   // Failed Hmd creation.
		  exit(1);
	  }
  }
  else
  {
	  // Get some details about the HMD
	  resolution = hmd->Resolution;
  }

  //TODO: find a place where to check for the status of the glFrameBuffer

  // Render texture initialization
  recommendedTex0Size = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left, hmd->DefaultEyeFov[0], 1.0f);
  recommendedTex1Size = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right, hmd->DefaultEyeFov[1], 1.0f);
  renderTargetSize.w = recommendedTex0Size.w + recommendedTex1Size.w;
  renderTargetSize.h = max(recommendedTex0Size.h, recommendedTex1Size.h);

  ovrFovPort eyeFov[2] = { hmd->DefaultEyeFov[0], hmd->DefaultEyeFov[1] };

  eyeRenderViewport[0].Pos  = Vector2i(0, 0);
  eyeRenderViewport[0].Size = Sizei((renderTargetSize.w/2), renderTargetSize.h);
  eyeRenderViewport[1].Pos = Vector2i((renderTargetSize.w + 1) / 2, 0);
  eyeRenderViewport[1].Size = eyeRenderViewport[0].Size;

  ci::gl::Fbo::Format format;
  format.enableDepthBuffer();
  format.setSamples(16);
  m_HMDFbo = ci::gl::Fbo(renderTargetSize.w, renderTargetSize.h, format);
  std::cout << "Init FBO size: " << m_HMDFbo.getSize() << std::endl;

  eyeTexture[0].OGL.Header.API = ovrRenderAPI_OpenGL;
  eyeTexture[0].OGL.Header.RenderViewport = eyeRenderViewport[0];
  eyeTexture[0].OGL.TexId = m_HMDFbo.getTexture().getId();
  eyeTexture[0].OGL.Header.TextureSize = renderTargetSize;
  
  eyeTexture[1] = eyeTexture[0];
  eyeTexture[1].OGL.Header.RenderViewport = eyeRenderViewport[1];

  ovrGLConfig cfg;
  cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
  cfg.OGL.Header.RTSize = renderTargetSize;
  cfg.OGL.Header.Multisample = 1;

  if (!(hmd->HmdCaps & ovrHmdCap_ExtendDesktop))
	  ovrHmd_AttachToWindow(hmd, m_HWND, NULL, NULL);
  
      cfg.OGL.Window = m_HWND;

  cfg.OGL.DC = NULL;


  // Configure rendering
  
  ovrHmd_ConfigureRendering(hmd, &cfg.Config,	ovrDistortionCap_Chromatic | 
												ovrDistortionCap_Vignette | 
												ovrDistortionCap_TimeWarp | 
												ovrDistortionCap_Overdrive, 
												eyeFov, eyeRenderDesc);

  ovrHmd_SetEnabledCaps(hmd,	ovrHmdCap_LowPersistence | 
								ovrHmdCap_DynamicPrediction);

  ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | 
								ovrTrackingCap_MagYawCorrection | 
								ovrTrackingCap_Position, 0);
								
  ////////////////////////////////////////////////////

  gl::disableAlphaBlending();
  gl::disableDepthWrite();
  gl::disableDepthRead();
  
  mFboDim				= FBO_DIM;
  mFboSize				= Vec2f( (float)mFboDim, (float)mFboDim );
  mFboBounds			= Area( 0, 0, mFboDim, mFboDim );
  mPositionFbos[0]	= gl::Fbo( mFboDim, mFboDim, mRgba16Format );
  mPositionFbos[1]	= gl::Fbo( mFboDim, mFboDim, mRgba16Format );
  mVelocityFbos[0]	= gl::Fbo( mFboDim, mFboDim, mRgba16Format );
  mVelocityFbos[1]	= gl::Fbo( mFboDim, mFboDim, mRgba16Format );
  
  setFboPositions( mPositionFbos[0] );
  setFboPositions( mPositionFbos[1] );
  setFboVelocities( mVelocityFbos[0] );
  setFboVelocities( mVelocityFbos[1] );
  
  initVbo();
}

void FlockingApp::setFboPositions( gl::Fbo fbo )
{	
  // FISH POSITION
  Surface32f posSurface( fbo.getTexture() );
  Surface32f::Iter it = posSurface.getIter();
  while( it.line() ){
    float y = (float)it.y()/(float)it.getHeight() - 0.5f;
    while( it.pixel() ){
      float per		= (float)it.x()/(float)it.getWidth();
      float angle		= per * (float)M_PI * 2.0f;
      float radius	= 100.0f;
      float cosA		= cos( angle );
      float sinA		= sin( angle );
      Vec3f p			= Vec3f( cosA, y, sinA ) * radius;
      
      it.r() = p.x;
      it.g() = p.y;
      it.b() = p.z;
      it.a() = Rand::randFloat( 0.7f, 1.0f );	// GENERAL EMOTIONAL STATE. 
    }
  }
  
  gl::Texture posTexture( posSurface );
  fbo.bindFramebuffer();
  gl::setMatricesWindow( mFboSize, false );
  gl::setViewport( mFboBounds );
  gl::draw( posTexture );
  fbo.unbindFramebuffer();
}

void FlockingApp::setFboVelocities( gl::Fbo fbo )
{
  // FISH VELOCITY
  Surface32f velSurface( fbo.getTexture() );
  Surface32f::Iter it = velSurface.getIter();
  while( it.line() ){
    while( it.pixel() ){
      float per		= (float)it.x()/(float)it.getWidth();
      float angle		= per * (float)M_PI * 2.0f;
      float cosA		= cos( angle );
      float sinA		= sin( angle );
      Vec3f p			= Vec3f( cosA, 0.0f, sinA );
      it.r() = p.x;
      it.g() = p.y;
      it.b() = p.z;
      it.a() = 1.0f;
    }
  }
  
  gl::Texture velTexture( velSurface );
  fbo.bindFramebuffer();
  gl::setMatricesWindow( mFboSize, false );
  gl::setViewport( mFboBounds );
  gl::draw( velTexture );
  fbo.unbindFramebuffer();
}

void FlockingApp::initVbo()
{
  gl::VboMesh::Layout layout;
  layout.setStaticPositions();
  layout.setStaticTexCoords2d();
  layout.setStaticNormals();
  
  int numVertices = mFboDim * mFboDim;
  // 5 points make up the pyramid
  // 8 triangles make up two pyramids
  // 3 points per triangle
  
  mVboMesh		= gl::VboMesh( numVertices * 16 * 3, 0, layout, GL_TRIANGLES );
  
  float s2 = 0.7f;
  float s1 = 1.0f;
  float s0 = 0.5f;
  Vec3f p0( 0.0f, 0.0f, 3.0f );
  // FRONT BODY
  Vec3f p1( -s0, 0.0f, 1.0f );	// left
  Vec3f p2( 0.0f,  s2, 1.0f );	// top
  Vec3f p3(  s0, 0.0f, 1.0f );	// right
  Vec3f p4( 0.0f, -s1, 1.0f );	// bottom
  // BACK BODY
  Vec3f p5( -s0, 0.0f, 0.0f );	// left
  Vec3f p6( 0.0f,  s2, 0.0f );	// top
  Vec3f p7(  s0, 0.0f, 0.0f );	// right
  Vec3f p8( 0.0f, -s2, 0.0f );	// bottom
  
  Vec3f p9( 0.0f, 0.0f, -4.0f );
  
  Vec3f n;
  Vec3f n0 = Vec3f( 0.0f, 0.0f, 1.0f );
  Vec3f n1 = Vec3f(-1.0f, 0.0f, 0.0f ).normalized();	// left
  Vec3f n2 = Vec3f( 0.0f, 1.0f, 0.0f ).normalized();	// top
  Vec3f n3 = Vec3f( 1.0f, 0.0f, 0.0f ).normalized();	// right
  Vec3f n4 = Vec3f( 0.0f,-1.0f, 0.0f ).normalized();	// bottom
  Vec3f n5 = Vec3f( 0.0f, 0.0f,-1.0f );
  
  vector<Vec3f>		positions;
  vector<Vec3f>		normals;
  vector<Vec2f>		texCoords;
  
  for( int x = 0; x < mFboDim; ++x ) {
    for( int y = 0; y < mFboDim; ++y ) {
      float u = (float)x/(float)mFboDim;
      float v = (float)y/(float)mFboDim;
      Vec2f t = Vec2f( u, v );
      
      // NOSE
      positions.push_back( p0 );
      positions.push_back( p1 );
      positions.push_back( p2 );
      texCoords.push_back( t );
      texCoords.push_back( t );
      texCoords.push_back( t );
      n = ( p0 + p1 + p2 ).normalized();
      normals.push_back( n );
      normals.push_back( n1 );
      normals.push_back( n2 );
      
      positions.push_back( p0 );
      positions.push_back( p2 );
      positions.push_back( p3 );
      texCoords.push_back( t );
      texCoords.push_back( t );
      texCoords.push_back( t );
      n = ( p0 + p2 + p3 ).normalized();
      normals.push_back( n );
      normals.push_back( n2 );
      normals.push_back( n3 );
      
      positions.push_back( p0 );
      positions.push_back( p3 );
      positions.push_back( p4 );
      texCoords.push_back( t );
      texCoords.push_back( t );
      texCoords.push_back( t );
      n = ( p0 + p3 + p4 ).normalized();
      normals.push_back( n );
      normals.push_back( n3 );
      normals.push_back( n4 );
      
      positions.push_back( p0 );
      positions.push_back( p4 );
      positions.push_back( p1 );
      texCoords.push_back( t );
      texCoords.push_back( t );
      texCoords.push_back( t );
      n = ( p0 + p4 + p1 ).normalized();
      normals.push_back( n );
      normals.push_back( n4 );
      normals.push_back( n1 );
      
      // MIDDLE
      positions.push_back( p1 );
      positions.push_back( p5 );
      positions.push_back( p6 );
      texCoords.push_back( t );
      texCoords.push_back( t );
      texCoords.push_back( t );
      n = ( p1 + p5 + p6 ).normalized();
      normals.push_back( n1 );
      normals.push_back( n1 );
      normals.push_back( n2 );
      
      positions.push_back( p1 );
      positions.push_back( p6 );
      positions.push_back( p2 );
      texCoords.push_back( t );
      texCoords.push_back( t );
      texCoords.push_back( t );
      n = ( p1 + p5 + p6 ).normalized();
      normals.push_back( n1 );
      normals.push_back( n2 );
      normals.push_back( n2 );
      
      positions.push_back( p2 );
      positions.push_back( p6 );
      positions.push_back( p7 );
      texCoords.push_back( t );
      texCoords.push_back( t );
      texCoords.push_back( t );
      n = ( p1 + p5 + p6 ).normalized();
      normals.push_back( n2 );
      normals.push_back( n2 );
      normals.push_back( n3 );
      
      positions.push_back( p2 );
      positions.push_back( p7 );
      positions.push_back( p3 );
      texCoords.push_back( t );
      texCoords.push_back( t );
      texCoords.push_back( t );
      n = ( p1 + p5 + p6 ).normalized();
      normals.push_back( n2 );
      normals.push_back( n3 );
      normals.push_back( n3 );
      
      positions.push_back( p3 );
      positions.push_back( p7 );
      positions.push_back( p8 );
      texCoords.push_back( t );
      texCoords.push_back( t );
      texCoords.push_back( t );
      n = ( p1 + p5 + p6 ).normalized();
      normals.push_back( n3 );
      normals.push_back( n3 );
      normals.push_back( n4 );
      
      positions.push_back( p3 );
      positions.push_back( p8 );
      positions.push_back( p4 );
      texCoords.push_back( t );
      texCoords.push_back( t );
      texCoords.push_back( t );
      n = ( p1 + p5 + p6 ).normalized();
      normals.push_back( n3 );
      normals.push_back( n4 );
      normals.push_back( n4 );
      
      positions.push_back( p4 );
      positions.push_back( p8 );
      positions.push_back( p5 );
      texCoords.push_back( t );
      texCoords.push_back( t );
      texCoords.push_back( t );
      n = ( p1 + p5 + p6 ).normalized();
      normals.push_back( n4 );
      normals.push_back( n4 );
      normals.push_back( n1 );
      
      positions.push_back( p4 );
      positions.push_back( p5 );
      positions.push_back( p1 );
      texCoords.push_back( t );
      texCoords.push_back( t );
      texCoords.push_back( t );
      n = ( p1 + p5 + p6 ).normalized();
      normals.push_back( n4 );
      normals.push_back( n1 );
      normals.push_back( n1 );
      
      // TAIL
      positions.push_back( p9 );
      positions.push_back( p5 );
      positions.push_back( p8 );
      texCoords.push_back( t );
      texCoords.push_back( t );
      texCoords.push_back( t );
      n = ( p5 + p1 + p4 ).normalized();
      normals.push_back( n );
      normals.push_back( n1 );
      normals.push_back( n4 );
      
      positions.push_back( p9 );
      positions.push_back( p6 );
      positions.push_back( p5 );
      texCoords.push_back( t );
      texCoords.push_back( t );
      texCoords.push_back( t );
      n = ( p5 + p2 + p1 ).normalized();
      normals.push_back( n );
      normals.push_back( n2 );
      normals.push_back( n1 );
      
      positions.push_back( p9 );
      positions.push_back( p7 );
      positions.push_back( p6 );
      texCoords.push_back( t );
      texCoords.push_back( t );
      texCoords.push_back( t );
      n = ( p5 + p3 + p2 ).normalized();
      normals.push_back( n );
      normals.push_back( n3 );
      normals.push_back( n2 );
      
      positions.push_back( p9 );
      positions.push_back( p8 );
      positions.push_back( p7 );
      texCoords.push_back( t );
      texCoords.push_back( t );
      texCoords.push_back( t );
      n = ( p5 + p4 + p3 ).normalized();
      normals.push_back( n );
      normals.push_back( n4 );
      normals.push_back( n3 );
      
      
    }
  }
  
  mVboMesh.bufferPositions( positions );
  mVboMesh.bufferTexCoords2d( 0, texCoords );
  mVboMesh.bufferNormals( normals );
  mVboMesh.unbindBuffers();
}

void FlockingApp::mouseDown( MouseEvent event )
{
  mMouseDownPos = event.getPos();
  mMousePressed = true;
  mMouseOffset = Vec2f::zero();
}

void FlockingApp::mouseUp( MouseEvent event )
{
  mMousePressed = false;
  mMouseOffset = Vec2f::zero();
}

void FlockingApp::mouseMove( MouseEvent event )
{
  mMousePos = event.getPos();
}

void FlockingApp::mouseDrag( MouseEvent event )
{
  mouseMove( event );
  mMouseOffset = ( mMousePos - mMouseDownPos );
}

void FlockingApp::keyDown( KeyEvent event )
{
  if( event.getChar() == 'r' ){
    initialize();
  } else if( event.getChar() == 'f' ){
    setFullScreen( !isFullScreen() );
  }
}

void FlockingApp::update()
{
  if( !mInitUpdateCalled ){
    mInitUpdateCalled = true;
  }
  
  { // LEAP
    std::lock_guard<std::mutex> lock( mMutex );
    const Leap::Frame frame = mLeapController->frame();
    // CONTROLLER
    mController->updateLeap( frame.hands() );
  }
  
  // CONTROLLER
  mController->update();
  
  // CAMERA
  if( mMousePressed ){
    mSpringCam.dragCam( ( mMouseOffset ) * 0.01f, ( mMouseOffset ).length() * 0.01f );
  }
  mSpringCam.update( 0.3f );
  
  gl::disableAlphaBlending();
  gl::disableDepthRead();
  gl::disableDepthWrite();
  gl::color( Color( 1, 1, 1 ) );

  drawIntoVelocityFbo();
  drawIntoPositionFbo();
  drawIntoFingerTipsFbo();
}


// FISH VELOCITY
void FlockingApp::drawIntoVelocityFbo()
{
  gl::setMatricesWindow( mFboSize, false );
  gl::setViewport( mFboBounds );
  
  mVelocityFbos[ mThisFbo ].bindFramebuffer();
  gl::clear( ColorA( 0, 0, 0, 0 ) );
  
  mPositionFbos[ mPrevFbo ].bindTexture( 0 );
  mVelocityFbos[ mPrevFbo ].bindTexture( 1 );
  mFingerTipsFbo.bindTexture( 2 );
  
  mVelocityShader.bind();
  mVelocityShader.uniform( "positionTex", 0 );
  mVelocityShader.uniform( "velocityTex", 1 );
  mVelocityShader.uniform( "lanternsTex", 2 );
  mVelocityShader.uniform( "numLights", (float)mController->mNumTips );
  mVelocityShader.uniform( "invNumLights", 1.0f/(float)MAX_TIPS );
  mVelocityShader.uniform( "invNumLightsHalf", 1.0f/(float)MAX_TIPS * 0.5f );
  mVelocityShader.uniform( "att", 1.015f );
  mVelocityShader.uniform( "fboDim", mFboDim );
  mVelocityShader.uniform( "invFboDim", 1.0f/(float)mFboDim );
  float p = mPerlin.fBm( (float)(getElapsedSeconds()) * 0.01f ) * 2.0f + 1.0f;
  mVelocityShader.uniform( "sphereRadius", 100.0f * p );
  mVelocityShader.uniform( "dt", mController->mTimeAdjusted );
  gl::drawSolidRect( mFboBounds );
  mVelocityShader.unbind();
  
  mVelocityFbos[ mThisFbo ].unbindFramebuffer();
}

// FISH POSITION
void FlockingApp::drawIntoPositionFbo()
{	
  gl::setMatricesWindow( mFboSize, false );
  gl::setViewport( mFboBounds );
  
  mPositionFbos[ mThisFbo ].bindFramebuffer();
  mPositionFbos[ mPrevFbo ].bindTexture( 0 );
  mVelocityFbos[ mThisFbo ].bindTexture( 1 );
  
  mPositionShader.bind();
  mPositionShader.uniform( "position", 0 );
  mPositionShader.uniform( "velocity", 1 );
  mPositionShader.uniform( "dt", mController->mTimeAdjusted );
  gl::drawSolidRect( mFboBounds );
  mPositionShader.unbind();
  
  mPositionFbos[ mThisFbo ].unbindFramebuffer();
}

void FlockingApp::draw()
{
  if( !mInitUpdateCalled ){
    return;
  }
  gl::clear( ColorA( 0.3f, 0.1f, 0.1f, 0.0f ), true );
  gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
  
  gl::setMatricesWindow( getWindowSize() );
  gl::setViewport( getWindowBounds() );
  
  gl::enableAlphaBlending();
  gl::enable( GL_TEXTURE_2D );
  gl::disableDepthRead();
  gl::disableDepthWrite();
  
  mBgTex.bind();
  gl::drawSolidRect( getWindowBounds() );
  
  gl::setMatrices( mSpringCam.getCam() );
  
  gl::enableDepthRead();
  gl::enableDepthWrite();
  
  // DRAW PARTICLES
  mPositionFbos[mPrevFbo].bindTexture( 0 );
  mPositionFbos[mThisFbo].bindTexture( 1 );
  mVelocityFbos[mThisFbo].bindTexture( 2 );
  mFingerTipsFbo.bindTexture( 3 );
  mShader.bind();
  mShader.uniform( "prevPosition", 0 );
  mShader.uniform( "currentPosition", 1 );
  mShader.uniform( "currentVelocity", 2 );
  mShader.uniform( "lightsTex", 3 );
  mShader.uniform( "numLights", (float)mController->mNumTips );
  mShader.uniform( "invNumLights", 1.0f/(float)MAX_TIPS );
  mShader.uniform( "invNumLightsHalf", 1.0f/(float)MAX_TIPS * 0.5f );
  mShader.uniform( "att", 1.15f );
  mShader.uniform( "eyePos", mSpringCam.mEye );
  gl::draw( mVboMesh );
  mShader.unbind();
  
  // DRAW LANTERN GLOWS
  gl::disableDepthWrite();
  gl::enableAdditiveBlending();
  gl::color( Color( 1, 1, 1 ) );
  mLanternGlowTex.bind();
  mController->drawLanternGlows( mSpringCam.mBillboardRight, mSpringCam.mBillboardUp );
  drawGlows();
  drawNebulas();
  drawBubbles();
  
  gl::disable( GL_TEXTURE_2D );
  gl::enableDepthWrite();
  gl::enableAdditiveBlending();
  gl::color( Color( 1.0f, 1.0f, 1.0f ) );
  
  // DRAW LANTERNS
  mLanternShader.bind();
  mLanternShader.uniform( "mvpMatrix", mSpringCam.mMvpMatrix );
  mLanternShader.uniform( "eyePos", mSpringCam.mEye );
  mController->drawLanterns( &mLanternShader );
  mLanternShader.unbind();

  // SCREEN SPACE
  gl::setMatricesWindow( getWindowSize() );
  gl::enableAlphaBlending();
  gl::enable( GL_TEXTURE_2D );
  gl::disableDepthRead();
  gl::disableDepthWrite();
  
  drawTitle();
  
//	if( false ){	// DRAW POSITION AND VELOCITY FBOS
//		gl::color( Color::white() );
//		gl::setMatricesWindow( getWindowSize() );
//		gl::enable( GL_TEXTURE_2D );
//		mPositionFbos[ mThisFbo ].bindTexture();
//		gl::drawSolidRect( Rectf( 5.0f, 5.0f, 105.0f, 105.0f ) );
//		
//		mPositionFbos[ mPrevFbo ].bindTexture();
//		gl::drawSolidRect( Rectf( 106.0f, 5.0f, 206.0f, 105.0f ) );
//		
//		mVelocityFbos[ mThisFbo ].bindTexture();
//		gl::drawSolidRect( Rectf( 5.0f, 106.0f, 105.0f, 206.0f ) );
//		
//		mVelocityFbos[ mPrevFbo ].bindTexture();
//		gl::drawSolidRect( Rectf( 106.0f, 106.0f, 206.0f, 206.0f ) );
//	}
  
  mThisFbo	= ( mThisFbo + 1 ) % 2;
  mPrevFbo	= ( mThisFbo + 1 ) % 2;
  
//	if( mSaveFrames ){
//		writeImage( getHomeDirectory() + "Flocking/" + toString( mNumSavedFrames ) + ".png", copyWindowSurface() );
//		mNumSavedFrames ++;
//	}
  
  

  
  if( getElapsedFrames()%60 == 0 ){
	  console() << "FPS = " << getAverageFps() << std::endl;
  }
}

void FlockingApp::drawGlows()
{
  mGlowTex.bind( 0 );
  mGlowShader.bind();
  mGlowShader.uniform( "glowTex", 0 );
  mController->drawGlows( &mGlowShader, mSpringCam.mBillboardRight, mSpringCam.mBillboardUp );
  mGlowShader.unbind();
}

void FlockingApp::drawNebulas()
{
  mNebulaTex.bind( 0 );
  mNebulaShader.bind();
  mNebulaShader.uniform( "nebulaTex", 0 );
  mController->drawNebulas( &mNebulaShader, mSpringCam.mBillboardRight, mSpringCam.mBillboardUp );
  mNebulaShader.unbind();
}

void FlockingApp::drawBubbles()
{
  mBubbleTex.bind( 0 );
  mGlowShader.bind();
  mGlowShader.uniform( "glowTex", 0 );
  mController->drawBubbles( &mGlowShader, mSpringCam.mBillboardRight, mSpringCam.mBillboardUp );
  mGlowShader.unbind();
}

void FlockingApp::drawTitle()
{
  float alpha = 1.0f - math<float>::max( (float)(getElapsedSeconds()) - 3.0f, 0.0f );
  
  if( alpha > 0.0f ){
    gl::color( ColorA( 1.0f, 1.0f, 1.0f, alpha ) );
    
    float w = (float)mTitleTex.getWidth();
    float h = (float)mTitleTex.getHeight();
    Rectf r = Rectf( getWindowCenter() - Vec2f( w/2, h/2 ), getWindowCenter() + Vec2f( w/2, h/2 ) );
    mTitleTex.bind();
    gl::drawSolidRect( r );
  }
}

// HOLDS DATA FOR LANTERNS AND PREDATORS
void FlockingApp::drawIntoFingerTipsFbo()
{
  std::vector<Vec3f> tipPs;	// Positions
  std::vector<float> tipRs;	// Radii
  std::vector<Color> tipCs;	// Colors
  std::vector<float> tipHs;	// Hues
  
  for( map<uint32_t,FingerTip>::iterator activeIt = mController->mActiveTips.begin(); activeIt != mController->mActiveTips.end(); ++activeIt ){
    FingerTip tip = activeIt->second;
    tipPs.push_back( tip.mPos );
    tipRs.push_back( tip.mRadius );
    tipCs.push_back( tip.mColor );
    tipHs.push_back( tip.mAggression );
  }
  
  Surface32f tipsSurface( mFingerTipsFbo.getTexture() );
  Surface32f::Iter it = tipsSurface.getIter();
  while( it.line() ){
    while( it.pixel() ){
      int index = it.x();
      
      if( it.y() == 0 ){ // set light position
        if( index < (int)tipPs.size() ){
          it.r() = tipPs[index].x;
          it.g() = tipPs[index].y;
          it.b() = tipPs[index].z;
          it.a() = tipRs[index];
        } else { // if the light shouldnt exist, put it way out there
          it.r() = 0.0f;
          it.g() = 0.0f;
          it.b() = 0.0f;
          it.a() = 1.0f;
        }
      } else {	// set light color
        if( index < (int)tipPs.size() ){
          it.r() = tipCs[index].r;
          it.g() = tipCs[index].g;
          it.b() = tipCs[index].b;
          it.a() = tipHs[index];
        } else { 
          it.r() = 0.0f;
          it.g() = 0.0f;
          it.b() = 0.0f;
          it.a() = 1.0f;
        }
      }
    }
  }

  mFingerTipsFbo.bindFramebuffer();
  gl::setMatricesWindow( mFingerTipsFbo.getSize(), false );
  gl::setViewport( mFingerTipsFbo.getBounds() );
  gl::draw( gl::Texture( tipsSurface ) );
  mFingerTipsFbo.unbindFramebuffer();
}


CINDER_APP_BASIC( FlockingApp, RendererGl )
