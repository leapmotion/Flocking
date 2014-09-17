#include "cinder/app/AppBasic.h"
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

#include "cinder/gl/gl.h"
#include "Resources.h"
//#include "HodginUtility.h"
#include "SpringCam.h"
#include "Room.h"
#include "Controller.h"

#include "LeapListener.h"
#include "Leap.h"
#include <iostream>

#define __glew_h__
#define __GLEW_H__
#define GLEW_STATIC

/**
#include "EigenTypes.h"
#include "OVR.h"
#define __glew_h__
#define __GLEW_H__
#define GLEW_STATIC
#include "GL/glew.h"

#define OVR_OS_WIN32

#include "OVR_CAPI_GL.h"
#include "Kernel/OVR_Math.h"
#include "SDL.h"
#include "SDL_syswm.h"

using namespace OVR;
*/

#include "SFMLController.h"
#include "GLController.h"
#include "OculusVR.h"

#if _WIN32
#include "Mirror.h"
#endif

#include <cassert>
#include <iostream>

#include "gl_glext_glu.h"

#include <memory>
#include <vector>


using namespace ci;
using namespace ci::app;
using namespace std;

#define          SDK_RENDER 1

#define APP_WIDTH		1280
#define APP_HEIGHT		720
#define FBO_DIM			50
#define P_FBO_DIM		5
#define MAX_TIPS		10

class FlockingApp : public AppBasic {
public:
  virtual void		prepareSettings(Settings *settings);
  virtual void		setup();
  virtual void		shutdown();
  void				adjustFboDim(int offset);
  void				initialize();
  void				setFboPositions(gl::Fbo fbo);
  void				setFboVelocities(gl::Fbo fbo);
  void				initVbo();
  virtual void		mouseDown(MouseEvent event);
  virtual void		mouseUp(MouseEvent event);
  virtual void		mouseMove(MouseEvent event);
  virtual void		mouseDrag(MouseEvent event);
  virtual void		keyDown(KeyEvent event);
  virtual void		update();
  void				updateLeap();
  void				drawIntoVelocityFbo();
  void				drawIntoPositionFbo();
  void				drawIntoFingerTipsFbo();
  void				drawGlows();
  void				drawNebulas();
  void				drawBubbles();
  void				drawTitle(const ci::Vec2i& window);
  virtual void		draw();
  void				    renderEyeView(int eyeIndex);

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW


  std::mutex		mMutex;

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
  bool        mInitialized;

  // OVR SDK //

  ci::Rectf m_curRect;
  Matrix4x4f m_curProjection;
  Matrix4x4f m_curModelView;

  Matrix4x4f m_mvpMatrix;
  Vector3f m_curEyePos;
  Vector3f m_billboardRight;
  Vector3f m_billboardUp;

  OculusVR m_Oculus;
  SFMLController m_SFMLController;
  GLController m_GLController;

  // For software window mirroring with Oculus
  std::thread mThread;
  HWND mirrorHwnd;
};

void FlockingApp::prepareSettings(Settings *settings) {
  settings->setWindowSize(APP_WIDTH, APP_HEIGHT);
  // settings->enableConsoleWindow();
}

void FlockingApp::setup() {
#if _WIN32
  mThread = std::thread(RunMirror, getRenderer()->getHwnd(), std::ref(mirrorHwnd));
#endif
  mInitUpdateCalled	= false;
  mInitialized = false;

  // LEAP
  mListener			= new LeapListener(&mMutex);
  mLeapController		= new Leap::Controller();
  mLeapController->addListener(*mListener);
  mLeapController->setPolicyFlags(Leap::Controller::POLICY_OPTIMIZE_HMD);

  // POSITION/VELOCITY FBOS
  mRgba16Format.setColorInternalFormat(GL_RGBA32F_ARB);
  mRgba16Format.setMinFilter(GL_NEAREST);
  mRgba16Format.setMagFilter(GL_NEAREST);
  mThisFbo				= 0;
  mPrevFbo				= 1;

  // LANTERNS
  mFingerTipsFbo			= gl::Fbo(MAX_TIPS, 2, mRgba16Format);

  // TEXTURE FORMAT
  gl::Texture::Format mipFmt;
  mipFmt.enableMipmapping(true);
  mipFmt.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
  mipFmt.setMagFilter(GL_LINEAR);

  // TEXTURES
  mLanternGlowTex			= gl::Texture(loadImage(loadResource(RES_LANTERNGLOW_PNG)));
  mGlowTex				= gl::Texture(loadImage(loadResource(RES_GLOW_PNG)));
  mNebulaTex				= gl::Texture(loadImage(loadResource(RES_NEBULA_PNG)));
  mBubbleTex				= gl::Texture(loadImage(loadResource(RES_BUBBLE_PNG)));
  mBgTex					= gl::Texture(loadImage(loadResource(RES_BG_PNG)));
  mTitleTex				= gl::Texture(loadImage(loadResource(RES_TITLE_PNG)));

  // LOAD SHADERS
  try {
    mVelocityShader		= gl::GlslProg(loadResource(RES_PASSTHRU_VERT),	loadResource(RES_VELOCITY_FRAG));
    mPositionShader		= gl::GlslProg(loadResource(RES_PASSTHRU_VERT),	loadResource(RES_POSITION_FRAG));
    mLanternShader		= gl::GlslProg(loadResource(RES_LANTERN_VERT),	loadResource(RES_LANTERN_FRAG));
    mRoomShader			= gl::GlslProg(loadResource(RES_ROOM_VERT),		loadResource(RES_ROOM_FRAG));
    mShader				= gl::GlslProg(loadResource(RES_VBOPOS_VERT),	loadResource(RES_VBOPOS_FRAG));
    mGlowShader			= gl::GlslProg(loadResource(RES_PASSTHRU_VERT),	loadResource(RES_GLOW_FRAG));
    mNebulaShader		= gl::GlslProg(loadResource(RES_PASSTHRU_VERT),	loadResource(RES_NEBULA_FRAG));
  } catch (gl::GlslProgCompileExc e) {
    std::cout << e.what() << std::endl;
    quit();
  }

  // CONTROLLER
  mController			= new Controller(MAX_TIPS);

  // PERLIN
  mPerlin				= Perlin(4);

  // MOUSE
  mMousePos			= Vec2f::zero();
  mMouseDownPos		= Vec2f::zero();
  mMouseOffset		= Vec2f::zero();
  mMousePressed		= false;


#if _WIN32
  HWND curRenderer =  getRenderer()->getHwnd();
  m_Oculus.SetHWND(curRenderer);
#endif

  m_Oculus.InitGlew();

  if (!m_Oculus.Init()) {
    throw std::runtime_error("Oculus initialization failed");
  }

  ovrHSWDisplayState hswDisplayState;
  ovrHmd_GetHSWDisplayState(m_Oculus.GetHMD(), &hswDisplayState);
  if (hswDisplayState.Displayed) {
    // Dismiss the warning
    ovrHmd_DismissHSWDisplay(m_Oculus.GetHMD());
  } else {
    // Detect a moderate tap on the side of the HMD.
    ovrTrackingState ts = ovrHmd_GetTrackingState(m_Oculus.GetHMD(), ovr_GetTimeInSeconds());
    if (ts.StatusFlags & ovrStatus_OrientationTracked) {
      const OVR::Vector3f v(ts.RawSensorData.Accelerometer.x,
                            ts.RawSensorData.Accelerometer.y,
                            ts.RawSensorData.Accelerometer.z);
      // Arbitrary value and representing moderate tap on the side of the DK2 Rift.
      if (v.LengthSq() > 250.f) {
        ovrHmd_DismissHSWDisplay(m_Oculus.GetHMD());
      }
    }
  }

  initialize();
  mInitialized = true;
}

void FlockingApp::shutdown() {
  if (mThread.joinable()) {
    PostMessage(mirrorHwnd, WM_CLOSE, 0, 0);
    mThread.join();
  }
}

void FlockingApp::initialize() {
  gl::disableAlphaBlending();
  gl::disableDepthWrite();
  gl::disableDepthRead();

  mFboDim				= FBO_DIM;
  mFboSize				= Vec2f((float)mFboDim, (float)mFboDim);
  mFboBounds			= Area(0, 0, mFboDim, mFboDim);
  mPositionFbos[0]	= gl::Fbo(mFboDim, mFboDim, mRgba16Format);
  mPositionFbos[1]	= gl::Fbo(mFboDim, mFboDim, mRgba16Format);
  mVelocityFbos[0]	= gl::Fbo(mFboDim, mFboDim, mRgba16Format);
  mVelocityFbos[1]	= gl::Fbo(mFboDim, mFboDim, mRgba16Format);

  setFboPositions(mPositionFbos[0]);
  setFboPositions(mPositionFbos[1]);
  setFboVelocities(mVelocityFbos[0]);
  setFboVelocities(mVelocityFbos[1]);

  initVbo();
}

void FlockingApp::setFboPositions(gl::Fbo fbo) {
  // FISH POSITION
  Surface32f posSurface(fbo.getTexture());
  Surface32f::Iter it = posSurface.getIter();
  while (it.line()) {
    float y = (float)it.y()/(float)it.getHeight() - 0.5f;
    while (it.pixel()) {
      float per		= (float)it.x()/(float)it.getWidth();
      float angle		= per * (float)M_PI * 2.0f;
      float radius	= 100.0f;
      float cosA		= cos(angle);
      float sinA		= sin(angle);
      Vec3f p			= Vec3f(cosA, y, sinA) * radius;

      it.r() = p.x;
      it.g() = p.y;
      it.b() = p.z;
      it.a() = Rand::randFloat(0.7f, 1.0f);	// GENERAL EMOTIONAL STATE.
    }
  }

  gl::Texture posTexture(posSurface);
  fbo.bindFramebuffer();
  gl::setMatricesWindow(mFboSize, false);
  gl::setViewport(mFboBounds);
  gl::draw(posTexture);
  fbo.unbindFramebuffer();
}

void FlockingApp::setFboVelocities(gl::Fbo fbo) {
  // FISH VELOCITY
  Surface32f velSurface(fbo.getTexture());
  Surface32f::Iter it = velSurface.getIter();
  while (it.line()) {
    while (it.pixel()) {
      float per		= (float)it.x()/(float)it.getWidth();
      float angle		= per * (float)M_PI * 2.0f;
      float cosA		= cos(angle);
      float sinA		= sin(angle);
      Vec3f p			= Vec3f(cosA, 0.0f, sinA);
      it.r() = p.x;
      it.g() = p.y;
      it.b() = p.z;
      it.a() = 1.0f;
    }
  }

  gl::Texture velTexture(velSurface);
  fbo.bindFramebuffer();
  gl::setMatricesWindow(mFboSize, false);
  gl::setViewport(mFboBounds);
  gl::draw(velTexture);
  fbo.unbindFramebuffer();
}

void FlockingApp::initVbo() {
  gl::VboMesh::Layout layout;
  layout.setStaticPositions();
  layout.setStaticTexCoords2d();
  layout.setStaticNormals();

  int numVertices = mFboDim * mFboDim;
  // 5 points make up the pyramid
  // 8 triangles make up two pyramids
  // 3 points per triangle

  mVboMesh		= gl::VboMesh(numVertices * 16 * 3, 0, layout, GL_TRIANGLES);

  float s2 = 0.7f;
  float s1 = 1.0f;
  float s0 = 0.5f;
  Vec3f p0(0.0f, 0.0f, 3.0f);
  // FRONT BODY
  Vec3f p1(-s0, 0.0f, 1.0f);	// left
  Vec3f p2(0.0f, s2, 1.0f);	// top
  Vec3f p3(s0, 0.0f, 1.0f);	// right
  Vec3f p4(0.0f, -s1, 1.0f);	// bottom
  // BACK BODY
  Vec3f p5(-s0, 0.0f, 0.0f);	// left
  Vec3f p6(0.0f, s2, 0.0f);	// top
  Vec3f p7(s0, 0.0f, 0.0f);	// right
  Vec3f p8(0.0f, -s2, 0.0f);	// bottom

  Vec3f p9(0.0f, 0.0f, -4.0f);

  Vec3f n;
  Vec3f n0 = Vec3f(0.0f, 0.0f, 1.0f);
  Vec3f n1 = Vec3f(-1.0f, 0.0f, 0.0f).normalized();	// left
  Vec3f n2 = Vec3f(0.0f, 1.0f, 0.0f).normalized();	// top
  Vec3f n3 = Vec3f(1.0f, 0.0f, 0.0f).normalized();	// right
  Vec3f n4 = Vec3f(0.0f, -1.0f, 0.0f).normalized();	// bottom
  Vec3f n5 = Vec3f(0.0f, 0.0f, -1.0f);

  vector<Vec3f>		positions;
  vector<Vec3f>		normals;
  vector<Vec2f>		texCoords;

  for (int x = 0; x < mFboDim; ++x) {
    for (int y = 0; y < mFboDim; ++y) {
      float u = (float)x/(float)mFboDim;
      float v = (float)y/(float)mFboDim;
      Vec2f t = Vec2f(u, v);

      // NOSE
      positions.push_back(p0);
      positions.push_back(p1);
      positions.push_back(p2);
      texCoords.push_back(t);
      texCoords.push_back(t);
      texCoords.push_back(t);
      n = (p0 + p1 + p2).normalized();
      normals.push_back(n);
      normals.push_back(n1);
      normals.push_back(n2);

      positions.push_back(p0);
      positions.push_back(p2);
      positions.push_back(p3);
      texCoords.push_back(t);
      texCoords.push_back(t);
      texCoords.push_back(t);
      n = (p0 + p2 + p3).normalized();
      normals.push_back(n);
      normals.push_back(n2);
      normals.push_back(n3);

      positions.push_back(p0);
      positions.push_back(p3);
      positions.push_back(p4);
      texCoords.push_back(t);
      texCoords.push_back(t);
      texCoords.push_back(t);
      n = (p0 + p3 + p4).normalized();
      normals.push_back(n);
      normals.push_back(n3);
      normals.push_back(n4);

      positions.push_back(p0);
      positions.push_back(p4);
      positions.push_back(p1);
      texCoords.push_back(t);
      texCoords.push_back(t);
      texCoords.push_back(t);
      n = (p0 + p4 + p1).normalized();
      normals.push_back(n);
      normals.push_back(n4);
      normals.push_back(n1);

      // MIDDLE
      positions.push_back(p1);
      positions.push_back(p5);
      positions.push_back(p6);
      texCoords.push_back(t);
      texCoords.push_back(t);
      texCoords.push_back(t);
      n = (p1 + p5 + p6).normalized();
      normals.push_back(n1);
      normals.push_back(n1);
      normals.push_back(n2);

      positions.push_back(p1);
      positions.push_back(p6);
      positions.push_back(p2);
      texCoords.push_back(t);
      texCoords.push_back(t);
      texCoords.push_back(t);
      n = (p1 + p5 + p6).normalized();
      normals.push_back(n1);
      normals.push_back(n2);
      normals.push_back(n2);

      positions.push_back(p2);
      positions.push_back(p6);
      positions.push_back(p7);
      texCoords.push_back(t);
      texCoords.push_back(t);
      texCoords.push_back(t);
      n = (p1 + p5 + p6).normalized();
      normals.push_back(n2);
      normals.push_back(n2);
      normals.push_back(n3);

      positions.push_back(p2);
      positions.push_back(p7);
      positions.push_back(p3);
      texCoords.push_back(t);
      texCoords.push_back(t);
      texCoords.push_back(t);
      n = (p1 + p5 + p6).normalized();
      normals.push_back(n2);
      normals.push_back(n3);
      normals.push_back(n3);

      positions.push_back(p3);
      positions.push_back(p7);
      positions.push_back(p8);
      texCoords.push_back(t);
      texCoords.push_back(t);
      texCoords.push_back(t);
      n = (p1 + p5 + p6).normalized();
      normals.push_back(n3);
      normals.push_back(n3);
      normals.push_back(n4);

      positions.push_back(p3);
      positions.push_back(p8);
      positions.push_back(p4);
      texCoords.push_back(t);
      texCoords.push_back(t);
      texCoords.push_back(t);
      n = (p1 + p5 + p6).normalized();
      normals.push_back(n3);
      normals.push_back(n4);
      normals.push_back(n4);

      positions.push_back(p4);
      positions.push_back(p8);
      positions.push_back(p5);
      texCoords.push_back(t);
      texCoords.push_back(t);
      texCoords.push_back(t);
      n = (p1 + p5 + p6).normalized();
      normals.push_back(n4);
      normals.push_back(n4);
      normals.push_back(n1);

      positions.push_back(p4);
      positions.push_back(p5);
      positions.push_back(p1);
      texCoords.push_back(t);
      texCoords.push_back(t);
      texCoords.push_back(t);
      n = (p1 + p5 + p6).normalized();
      normals.push_back(n4);
      normals.push_back(n1);
      normals.push_back(n1);

      // TAIL
      positions.push_back(p9);
      positions.push_back(p5);
      positions.push_back(p8);
      texCoords.push_back(t);
      texCoords.push_back(t);
      texCoords.push_back(t);
      n = (p5 + p1 + p4).normalized();
      normals.push_back(n);
      normals.push_back(n1);
      normals.push_back(n4);

      positions.push_back(p9);
      positions.push_back(p6);
      positions.push_back(p5);
      texCoords.push_back(t);
      texCoords.push_back(t);
      texCoords.push_back(t);
      n = (p5 + p2 + p1).normalized();
      normals.push_back(n);
      normals.push_back(n2);
      normals.push_back(n1);

      positions.push_back(p9);
      positions.push_back(p7);
      positions.push_back(p6);
      texCoords.push_back(t);
      texCoords.push_back(t);
      texCoords.push_back(t);
      n = (p5 + p3 + p2).normalized();
      normals.push_back(n);
      normals.push_back(n3);
      normals.push_back(n2);

      positions.push_back(p9);
      positions.push_back(p8);
      positions.push_back(p7);
      texCoords.push_back(t);
      texCoords.push_back(t);
      texCoords.push_back(t);
      n = (p5 + p4 + p3).normalized();
      normals.push_back(n);
      normals.push_back(n4);
      normals.push_back(n3);


    }
  }

  mVboMesh.bufferPositions(positions);
  mVboMesh.bufferTexCoords2d(0, texCoords);
  mVboMesh.bufferNormals(normals);
  mVboMesh.unbindBuffers();
}

void FlockingApp::mouseDown(MouseEvent event) {
  mMouseDownPos = event.getPos();
  mMousePressed = true;
  mMouseOffset = Vec2f::zero();
}

void FlockingApp::mouseUp(MouseEvent event) {
  mMousePressed = false;
  mMouseOffset = Vec2f::zero();
}

void FlockingApp::mouseMove(MouseEvent event) {
  mMousePos = event.getPos();
}

void FlockingApp::mouseDrag(MouseEvent event) {
  mouseMove(event);
  mMouseOffset = (mMousePos - mMouseDownPos);
}

void FlockingApp::keyDown(KeyEvent event) {
  if (event.getChar() == 'r') {
    initialize();
  } else if (event.getChar() == 'f') {
    setFullScreen(!isFullScreen());
  } else if (event.getChar() == ci::app::KeyEvent::KEY_ESCAPE) {
    quit();
  }
}

void FlockingApp::update() {
  if (!mInitialized) {
    return;
  }

  if (!mInitUpdateCalled) {
    mInitUpdateCalled = true;
  }


  const Matrix4x4f invRotation = m_Oculus.EyeRotation(0).inverse();

  {
    // LEAP
    std::lock_guard<std::mutex> lock(mMutex);
    const Leap::Frame frame = mLeapController->frame();
    // CONTROLLER
    mController->m_invRot = ci::Matrix44f(invRotation.data());
    mController->updateLeap(frame.hands());
  }

  // CONTROLLER
  mController->update();

  gl::disableAlphaBlending();
  gl::disableDepthRead();
  gl::disableDepthWrite();
  gl::color(Color(1, 1, 1));

  drawIntoVelocityFbo();
  drawIntoPositionFbo();
  drawIntoFingerTipsFbo();
}


// FISH VELOCITY
void FlockingApp::drawIntoVelocityFbo() {
  gl::setMatricesWindow(mFboSize, false);
  gl::setViewport(mFboBounds);

  mVelocityFbos[mThisFbo].bindFramebuffer();
  gl::clear(ColorA(0, 0, 0, 0));

  mPositionFbos[mPrevFbo].bindTexture(0);
  mVelocityFbos[mPrevFbo].bindTexture(1);
  mFingerTipsFbo.bindTexture(2);

  mVelocityShader.bind();
  mVelocityShader.uniform("positionTex", 0);
  mVelocityShader.uniform("velocityTex", 1);
  mVelocityShader.uniform("lanternsTex", 2);
  mVelocityShader.uniform("numLights", (float)mController->mNumTips);
  mVelocityShader.uniform("invNumLights", 1.0f/(float)MAX_TIPS);
  mVelocityShader.uniform("invNumLightsHalf", 1.0f/(float)MAX_TIPS * 0.5f);
  mVelocityShader.uniform("att", 1.015f);
  mVelocityShader.uniform("fboDim", mFboDim);
  mVelocityShader.uniform("invFboDim", 1.0f/(float)mFboDim);
  float p = mPerlin.fBm((float)(getElapsedSeconds()) * 0.01f) * 2.0f + 1.0f;
  mVelocityShader.uniform("sphereRadius", 100.0f * p);
  mVelocityShader.uniform("dt", mController->mTimeAdjusted);
  gl::drawSolidRect(mFboBounds);
  mVelocityShader.unbind();

  mVelocityFbos[mThisFbo].unbindFramebuffer();
}

// FISH POSITION
void FlockingApp::drawIntoPositionFbo() {
  gl::setMatricesWindow(mFboSize, false);
  gl::setViewport(mFboBounds);

  mPositionFbos[mThisFbo].bindFramebuffer();
  mPositionFbos[mPrevFbo].bindTexture(0);
  mVelocityFbos[mThisFbo].bindTexture(1);

  mPositionShader.bind();
  mPositionShader.uniform("position", 0);
  mPositionShader.uniform("velocity", 1);
  mPositionShader.uniform("dt", mController->mTimeAdjusted);
  gl::drawSolidRect(mFboBounds);
  mPositionShader.unbind();

  mPositionFbos[mThisFbo].unbindFramebuffer();
}

inline cinder::Vec3f ToVec3f(const Vector3f& v) { return cinder::Vec3f(v.x(), v.y(), v.z()); }
inline cinder::Matrix44f ToMat44f(const Matrix4x4f& m) {
  ci::Matrix44f mat;
  for (int i=0; i<4; i++) {
    for (int j=0; j<4; j++) {
      mat.at(i, j) = m(i, j);
    }
  }
  return mat;
}


void FlockingApp::renderEyeView(int eyeIndex) {
  const ci::Vec2i window = ci::Vec2i(m_curRect.getSize().x, m_curRect.getSize().y);

  gl::setMatricesWindow(window);
  glViewport(m_curRect.x1, m_curRect.y1, m_curRect.getSize().x, m_curRect.getSize().y);

  gl::enableAlphaBlending();
  gl::enable(GL_TEXTURE_2D);
  gl::disableDepthRead();
  gl::disableDepthWrite();

  // mBgTex.bind();
  // gl::drawSolidRect(ci::Rectf(0, 0, m_curRect.getSize().x, m_curRect.getSize().y));
  // mBgTex.unbind();
  glActiveTexture(0);

  glViewport(m_curRect.x1, m_curRect.y1, m_curRect.getSize().x, m_curRect.getSize().y);

  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(m_curProjection.data());

  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(m_curModelView.data());

  gl::enableDepthRead();
  gl::enableDepthWrite();

  // DRAW PARTICLES
  mPositionFbos[mPrevFbo].bindTexture(0);
  mPositionFbos[mThisFbo].bindTexture(1);
  mVelocityFbos[mThisFbo].bindTexture(2);
  mFingerTipsFbo.bindTexture(3);
  mShader.bind();
  mShader.uniform("prevPosition", 0);
  mShader.uniform("currentPosition", 1);
  mShader.uniform("currentVelocity", 2);
  mShader.uniform("lightsTex", 3);
  mShader.uniform("numLights", (float)mController->mNumTips);
  mShader.uniform("invNumLights", 1.0f/(float)MAX_TIPS);
  mShader.uniform("invNumLightsHalf", 1.0f/(float)MAX_TIPS * 0.5f);
  mShader.uniform("att", 1.15f);
  mShader.uniform("eyePos", ToVec3f(m_curEyePos));
  gl::draw(mVboMesh);
  mShader.unbind();

  mFingerTipsFbo.unbindTexture();
  mVelocityFbos[mThisFbo].unbindTexture();
  mPositionFbos[mThisFbo].unbindTexture();
  mPositionFbos[mPrevFbo].unbindTexture();

  // DRAW LANTERN GLOWS
  gl::disableDepthWrite();
  gl::enableAdditiveBlending();
  gl::color(Color(1, 1, 1));
  mLanternGlowTex.bind();
  mController->drawLanternGlows(ToVec3f(m_billboardRight), ToVec3f(m_billboardUp));
  mLanternGlowTex.unbind();
  drawGlows();
  drawNebulas();
  drawBubbles();

  gl::disable(GL_TEXTURE_2D);
  gl::enableDepthWrite();
  gl::enableAdditiveBlending();
  gl::color(Color(1.0f, 1.0f, 1.0f));

  // DRAW LANTERNS
  mLanternShader.bind();
  mLanternShader.uniform("mvpMatrix", ToMat44f(m_mvpMatrix));
  mLanternShader.uniform("eyePos", ToVec3f(m_curEyePos));
  mController->drawLanterns(&mLanternShader);
  mLanternShader.unbind();

  // SCREEN SPACE
  //drawTitle(window);
  gl::disableDepthRead();
  gl::disableDepthWrite();

  if (false) {	// DRAW POSITION AND VELOCITY FBOS
    gl::color(Color::white());
    gl::setMatricesWindow(window);
    gl::enable(GL_TEXTURE_2D);
    mPositionFbos[ mThisFbo ].bindTexture();
    gl::drawSolidRect(Rectf(5.0f, 5.0f, 105.0f, 105.0f));

    mPositionFbos[ mPrevFbo ].bindTexture();
    gl::drawSolidRect(Rectf(106.0f, 5.0f, 206.0f, 105.0f));

    mVelocityFbos[ mThisFbo ].bindTexture();
    gl::drawSolidRect(Rectf(5.0f, 106.0f, 105.0f, 206.0f));

    mVelocityFbos[ mPrevFbo ].bindTexture();
    gl::drawSolidRect(Rectf(106.0f, 106.0f, 206.0f, 206.0f));
  }

  if (getElapsedFrames() % 60 == 0) {
    console() << "FPS = " << getAverageFps() << std::endl;
  }
}

Matrix4x4f TranslationMatrix(const Vector3f& translation) {
  Matrix4x4f mat = Matrix4x4f::Identity();
  mat(0, 3) = translation[0];
  mat(1, 3) = translation[1];
  mat(2, 3) = translation[2];
  return mat;
}

void FlockingApp::draw() {
  if (!mInitUpdateCalled) {
    return;
  }
  m_Oculus.BeginFrame();

  gl::clear(ColorA(0.0f, 0.12f, 0.18f, 0.0f), true);
  // gl::clear(ColorA(0.0f, 0.0f, 0.0f, 0.0f), true);
  gl::color(ColorA(1.0f, 1.0f, 1.0f, 1.0f));

  // Shared render target eye rendering; set up RT once for both eyes.
  for (int eyeIndex = ovrEye_Count - 1; eyeIndex >= 0; --eyeIndex) {

    // Best setting to align with 3D only:                -84 & 1.0x      ( = 499.5f)
    // Best setting to align with peripheral passthrough:   0 & 1.6x      ( = 312.0f)
    // Best setting to align with dragonfly passthrough:    0 & 1.0x      ( = 499.5f)
    // Of course, in the peripheral case, by bumping the shift to 0, we may be able to turn the magnifier slightly down (somewhere between 1 and 1.6).

    const float LEAP_SCALE = 0.01f; // mm (Note the scale-down by 0.2x that was done in Controller.cpp)
    const float OCULUS_SCALE = 1.0f; // meters
    const float MULTIPLIER = 0.5f*(OCULUS_SCALE/LEAP_SCALE - 1.0f);

    const ovrRecti& rect = m_Oculus.EyeViewport(eyeIndex);
    m_curProjection = m_Oculus.EyeProjection(eyeIndex);
    m_curProjection.block<4, 3>(0, 0) *= LEAP_SCALE/OCULUS_SCALE;
    m_curModelView = m_Oculus.EyeView(eyeIndex);
    m_curModelView.block<3, 1>(0, 3) += MULTIPLIER*(m_curModelView.block<3, 1>(0, 3)
                                        - m_Oculus.EyeView(1 - eyeIndex).block<3, 1>(0, 3));
    m_curEyePos = Vector3f::Zero();

    m_mvpMatrix = m_curProjection * m_curModelView;

    m_billboardRight = m_curModelView.row(0).head<3>();
    m_billboardUp = m_curModelView.row(1).head<3>();

    m_curRect = ci::Rectf(rect.Pos.x, rect.Pos.y, rect.Pos.x + rect.Size.w, rect.Pos.y + rect.Size.h);

    renderEyeView(eyeIndex);
  }

  mThisFbo = (mThisFbo + 1) % 2;
  mPrevFbo = (mThisFbo + 1) % 2;

  m_Oculus.EndFrame();

  static bool first = true;
  if (first && ci::app::getElapsedSeconds() > 2.5) {
    initialize();
    first = false;
  }
}

void FlockingApp::drawGlows() {
  mGlowTex.bind();
  mGlowShader.bind();
  mGlowShader.uniform("glowTex", 0);
  mController->drawGlows(&mGlowShader, ToVec3f(m_billboardRight), ToVec3f(m_billboardUp));
  mGlowShader.unbind();
  mGlowTex.unbind();
  glActiveTexture(0);
}

void FlockingApp::drawNebulas() {
  mNebulaTex.bind();
  mNebulaShader.bind();
  mNebulaShader.uniform("nebulaTex", 0);
  mController->drawNebulas(&mNebulaShader, ToVec3f(m_billboardRight), ToVec3f(m_billboardUp));
  mNebulaShader.unbind();
  mNebulaTex.unbind();
  glActiveTexture(0);
}

void FlockingApp::drawBubbles() {
  mBubbleTex.bind();
  mGlowShader.bind();
  mGlowShader.uniform("glowTex", 0);
  mController->drawBubbles(&mGlowShader, ToVec3f(m_billboardRight), ToVec3f(m_billboardUp));
  mGlowShader.unbind();
  mBubbleTex.unbind();
  glActiveTexture(0);
}

void FlockingApp::drawTitle(const ci::Vec2i& window) {
  gl::setMatricesWindow(window);
  gl::enableAlphaBlending();
  gl::enable(GL_TEXTURE_2D);
  gl::disableDepthRead();
  gl::disableDepthWrite();

  float alpha = 1.0f - math<float>::max((float)(getElapsedSeconds()) - 3.0f, 0.0f);

  if (alpha > 0.0f) {
    gl::color(ColorA(1.0f, 1.0f, 1.0f, alpha));

    float w = (float)mTitleTex.getWidth();
    float h = (float)mTitleTex.getHeight();
    Rectf r = Rectf(Vec2f(0, 0), window);
    mTitleTex.bind();
    gl::drawSolidRect(r);
  }
}

// HOLDS DATA FOR LANTERNS AND PREDATORS
void FlockingApp::drawIntoFingerTipsFbo() {
  std::vector<Vec3f> tipPs;	// Positions
  std::vector<float> tipRs;	// Radii
  std::vector<Color> tipCs;	// Colors
  std::vector<float> tipHs;	// Hues

  for (map<uint32_t,FingerTip>::iterator activeIt = mController->mActiveTips.begin(); activeIt != mController->mActiveTips.end(); ++activeIt) {
    FingerTip tip = activeIt->second;
    tipPs.push_back(tip.mPos);
    tipRs.push_back(tip.mRadius);
    tipCs.push_back(tip.mColor);
    tipHs.push_back(tip.mAggression);
  }

  Surface32f tipsSurface(mFingerTipsFbo.getTexture());
  Surface32f::Iter it = tipsSurface.getIter();
  while (it.line()) {
    while (it.pixel()) {
      int index = it.x();

      if (it.y() == 0) { // set light position
        if (index < (int)tipPs.size()) {
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
        if (index < (int)tipPs.size()) {
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
  gl::setMatricesWindow(mFingerTipsFbo.getSize(), false);
  gl::setViewport(mFingerTipsFbo.getBounds());
  gl::draw(gl::Texture(tipsSurface));
  mFingerTipsFbo.unbindFramebuffer();
  glActiveTexture(0);
}


CINDER_APP_BASIC(FlockingApp, RendererGl)
