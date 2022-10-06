/**

 Eric Renaud-Houde - August 2014

 This sample illustrates common shadow mapping techniques.

 Overview ~

 A first pass stores the scene's depth information (from the light's POV)
 into a FBO.  When the shaded scene is rendered, a depth test is performed on
 each fragment. In the light's projective space, a fragment whose depth is
 greater than that of the shadow map must be occluded: it is shadowed.

 Common problems - Tradeoffs ~

 Aliasing: Other than increasing the resolution of the depth map, additionnal
 techniques can be used to soften the shadow edges. We demonstrate
 percentage-closer filtering (PCF) and random sampling. Note that sometimes
 lower resolution on the shadow map may help soften/blur the shadow.

 Surface acne/self-shadowing: Also occurring with traditional ray-tracing, this
 surface noise occurs on false depth tests (due to imprecision of a fragment's
 depth). Various offsets (such as glPolygonOffset & z-offsets in the light's
 projective space) can be used to prevent this problem.

 Peter Panning: The shadows don't reach the objects that cast them. This
 problem occurs when the offsets are too large. Offsets must be tweaked
 carefully to avoid problems on either end.

 Sampling noise: The random sampling method exhibits noise (which should
 still be less visually objectionable than aliasing). This is due to a low
 number of samples. More advanced GPU techniques allow one to increase this
 sample count.

 References ~

 OpenGL 4.0 Shading Language Cookbook by David Wolff
 https://github.com/daw42/glslcookbook
 Tutorial 16 : Shadow mapping
 http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
 Common Techniques to Improve Shadow Depth Maps
 http://msdn.microsoft.com/en-us/library/windows/desktop/ee416324(v=vs.85).aspx
 Soft Shadow Mapping
 http://codeflow.org/entries/2013/feb/15/soft-shadow-mapping/

 */


#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/GeomIo.h"
#include "cinder/Rand.h"
#include "cinder/CameraUi.h"
#include "cinder/Log.h"
#include "cinder/Color.h"
#if ! defined( CINDER_GL_ES )
#include "cinder/CinderImGui.h"
#endif
#include "glm/gtx/euler_angles.hpp"

#include "Fracture.h"
using namespace ci;
using namespace ci::app;

typedef std::shared_ptr<class ShadowMap> ShadowMapRef;

class FPhysics;
class FScene;
class ShadowMap {
public:
	static ShadowMapRef create(int size) { return ShadowMapRef(new ShadowMap{ size }); }
	ShadowMap(int size)
	{
		reset(size);
	}

	void reset(int size)
	{
		gl::Texture2d::Format depthFormat;
		depthFormat.setInternalFormat(GL_DEPTH_COMPONENT32F);
		depthFormat.setMagFilter(GL_LINEAR);
		depthFormat.setMinFilter(GL_LINEAR);
		depthFormat.setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		depthFormat.setCompareMode(GL_COMPARE_REF_TO_TEXTURE);
		depthFormat.setCompareFunc(GL_LEQUAL);
		mTextureShadowMap = gl::Texture2d::create(size, size, depthFormat);

		gl::Fbo::Format fboFormat;
		fboFormat.attachment(GL_DEPTH_ATTACHMENT, mTextureShadowMap);
		mShadowMap = gl::Fbo::create(size, size, fboFormat);
	}

	const gl::FboRef& getFbo() const { return mShadowMap; }
	const gl::Texture2dRef& getTexture() const { return mTextureShadowMap; }

	float					getAspectRatio() const { return mShadowMap->getAspectRatio(); }
	ivec2					getSize() const { return mShadowMap->getSize(); }
private:
	gl::FboRef				mShadowMap;
	gl::Texture2dRef		mTextureShadowMap;
};

struct LightData {
	bool						toggleViewpoint;
	float						distanceRadius;
	float						fov;
	CameraPersp					camera;
	vec3						viewpoint;
	vec3						target;
};

class Application : public App {
public:
	~Application();
	void setup() override;
	void update() override;
	void draw() override;

	void keyDown(KeyEvent event) override;
private:
	void drawScene(float spinAngle, const gl::GlslProgRef& glsl = nullptr);

	float						mFrameRate;
	CameraPersp					mCamera;
	CameraUi					mCamUi;

	gl::BatchRef				mTeapot, mTeapotShadowed;
	gl::BatchRef				mSphere, mSphereShadowed;
	gl::BatchRef				mGround, mGroundShadowed;
	std::vector<std::pair<mat4, vec3>>	mTransforms;


	gl::GlslProgRef				mShadowShader;
	ShadowMapRef				mShadowMap;
	int							mShadowMapSize;
	bool						mOnlyShadowmap;

	LightData					mLight;

	int							mShadowTechnique;

	float						mDepthBias;
	bool						mEnableNormSlopeOffset;
	float						mRandomOffset;
	int							mNumRandomSamples;
	float						mPolygonOffsetFactor, mPolygonOffsetUnits;

	FPhysics* m_pFPhysics=nullptr;
	FScene* m_pFScene = nullptr;
};