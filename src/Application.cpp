#include "Application.h"
#include "FPhysics.h"
#include "FScene.h"
Application::~Application()
{
	FRELEASE(m_pFPhysics);
}

void Application::setup()
{
	m_pFPhysics = new FPhysics();
	m_pFPhysics->Init();

	m_pFScene = new FScene();
	m_pFScene->Init();
	m_pFScene->SetSimulateState(true);
	m_pFPhysics->AddScene(m_pFScene);

	Rand::randomize();
	
	mFrameRate				= 0;
	mShadowMapSize			= 2048;
	
	mLight.distanceRadius	= 100.0f;
	mLight.viewpoint		= vec3( mLight.distanceRadius );
	mLight.fov				= 10.0f;
	mLight.target			= vec3( 0 );
	mLight.toggleViewpoint	= false;
	
	mShadowTechnique		= 1;
	mDepthBias				= -0.0005f;
	mRandomOffset			= 1.2f;
	mNumRandomSamples		= 32;
	mEnableNormSlopeOffset	= false;
	mOnlyShadowmap			= false;
	mPolygonOffsetFactor	= mPolygonOffsetUnits = 3.0f;
	
	Platform::get()->addAssetDirectory(".");

	try {
#if defined( CINDER_GL_ES )
		mShadowShader	= gl::GlslProg::create( loadAsset( "shadow_mapping_es3.vert"), loadAsset("shadow_mapping_es3.frag") );
#else
		mShadowShader	= gl::GlslProg::create( loadAsset( "shadow_mapping.vert"), loadAsset("shadow_mapping.frag") );
#endif
	} catch ( const gl::GlslProgCompileExc& exc ) {
		console() << "Shader failed to load: " << exc.what() << std::endl;
	}
	
	mShadowMap		= ShadowMap::create( mShadowMapSize );
	mLight.camera.setPerspective( mLight.fov, mShadowMap->getAspectRatio(), 0.5, 500.0 );
	
	ImGui::Initialize();
	
	auto positionGlsl = gl::getStockShader( gl::ShaderDef() );
	
	auto teapot = gl::VboMesh::create( geom::Cube().size(1,1,1) );
	mTeapot = gl::Batch::create( teapot, positionGlsl );
	mTeapotShadowed = gl::Batch::create( teapot, mShadowShader );
	
	auto sphere = gl::VboMesh::create( geom::Sphere().radius(0.1) );
	mSphere = gl::Batch::create( sphere, positionGlsl );
	mSphereShadowed = gl::Batch::create( sphere, mShadowShader );
		
	auto plane = gl::VboMesh::create(geom::Plane().normal(glm::vec3(0,1,0)));
	mGround= gl::Batch::create(plane, positionGlsl);
	mGroundShadowed = gl::Batch::create(plane, mShadowShader);

	for ( size_t i = 0; i < 10; ++i ) {
		vec3 v( 25.0f * randVec3() );
		mat4 m{};
		m *= translate( v );
		m *= scale( vec3( 6 * ( randFloat() + 1.1f ) ) );
		m *= rotate( 2 * glm::pi<float>() * randFloat(), randVec3() );
		mTransforms.emplace_back( m, randVec3() );
	}
	
	gl::enableDepthRead();
	gl::enableDepthWrite();

	mCamera.setFov( 30.0f );
	mCamera.setAspectRatio( getWindowAspectRatio() );
	mCamUi = CameraUi( &mCamera, getWindow() );
}

void Application::update()
{
	m_pFPhysics->Update();

	ImGui::Begin( "Settings" );
	ImGui::Text( "Framerate: %f", mFrameRate );
	ImGui::Separator();
	ImGui::Checkbox( "Light viewpoint", &mLight.toggleViewpoint );
	ImGui::DragFloat( "Light distance radius", &mLight.distanceRadius, 1.0f, 0.0f, 450.0f );
	ImGui::Checkbox( "Render only shadow map", &mOnlyShadowmap );
	ImGui::Separator();
	std::vector<std::string> techniques = { "Hard", "PCF3x3", "PCF4x4", "Random" };
	ImGui::Combo( "Technique", &mShadowTechnique, techniques );
	ImGui::Separator();
	ImGui::DragFloat( "Polygon offset factor", &mPolygonOffsetFactor, 0.025f, 0.0f );
	ImGui::DragFloat( "Polygon offset units", &mPolygonOffsetUnits, 0.025f, 0.0f );
	if( ImGui::DragInt( "Shadow map size", &mShadowMapSize, 16, 16, 2048 ) ) {
		mShadowMap->reset( mShadowMapSize );
	};
	ImGui::DragFloat( "Depth bias", &mDepthBias, 0.00001f, -1.0f, 0.0f, "%.5f" );
	ImGui::Text( "(PCF radius is const: tweak in shader.)" );
	ImGui::Separator();
	ImGui::Text( "Random sampling params" );
	ImGui::DragFloat( "Offset radius", &mRandomOffset, 0.05f, 0.0f );
	ImGui::Checkbox( "Auto normal slope offset", &mEnableNormSlopeOffset );
	ImGui::DragInt( "Num samples", &mNumRandomSamples, 1.0f, 1, 256 );
	ImGui::End();

	float e	= (float) getElapsedSeconds();
	float c = cos( e );
	float s	= sin( e );
	
	for ( auto& transform : mTransforms ) {
		transform.first *= orientate4( vec3( c, s, -c ) * 0.01f );
	}
	
	mLight.viewpoint.x = mLight.distanceRadius * sin( 0.25f * e );
	mLight.viewpoint.z = mLight.distanceRadius * cos( 0.25f * e );
	mLight.camera.lookAt( mLight.viewpoint, mLight.target );
	mFrameRate = getAverageFps();
}

void Application::drawScene( float spinAngle, const gl::GlslProgRef& shadowGlsl )
{

	{
		gl::ScopedColor gray(Color(0.8f, 0.8f, 0.8f));
		gl::ScopedModelMatrix push;
		gl::scale(vec3(4000));

		if (shadowGlsl) {
			shadowGlsl->uniform("uIsTeapot", false);
			mGroundShadowed->draw();
			shadowGlsl->uniform("uIsTeapot", true);
		}
		else {
			mGround->draw();
		}
	}
	{
		gl::ScopedColor red( Color( 0.98f, 0.22f, 0.10f ));
		gl::ScopedModelMatrix push;
		gl::scale( vec3(4) );
		
		if( shadowGlsl ) {
			shadowGlsl->uniform( "uIsTeapot", false );
			mSphereShadowed->draw();
			shadowGlsl->uniform( "uIsTeapot", true );
		}
		else {
			mSphere->draw();
		}
	}

	
	{
		gl::ScopedColor white( Color( 0.10f, 0.17f, 0.97f ) );
		for ( const auto& transform : mTransforms ) {
			gl::ScopedModelMatrix push;
			gl::scale( vec3(0.25) );
			gl::multModelMatrix( rotate( spinAngle, transform.second ) * transform.first );
			if( shadowGlsl )
				mTeapotShadowed->draw();
			else
				mTeapot->draw();
		}
	}
}

void Application::draw()
{
	gl::clear( Color( 0.07f, 0.05f, 0.1f ) );
	
	// Elapsed time called here: the scene must be absolutely identical on both renders!
	float spinAngle = 0.5f * (float) app::getElapsedSeconds();
	
	// Offset to help combat surface acne (self-shadowing)
	gl::enable( GL_POLYGON_OFFSET_FILL );
	glPolygonOffset( mPolygonOffsetFactor, mPolygonOffsetUnits );
	
	// Render scene into shadow map
	gl::setMatrices( mLight.camera );
	gl::viewport( mShadowMap->getSize() );
	{
		
		gl::ScopedFramebuffer bindFbo( mShadowMap->getFbo() );
		gl::clear();
		drawScene( spinAngle );
	}

	// Render shadowed scene
	gl::setMatrices( mLight.toggleViewpoint ? mLight.camera : mCamera );
	gl::viewport( toPixels( getWindowSize() ) );
	{
		gl::ScopedGlslProg bind( mShadowShader );
		gl::ScopedTextureBind texture( mShadowMap->getTexture() );
		
		mShadowShader->uniform( "uShadowMap", 0 );
		mShadowShader->uniform( "uShadowMatrix", mLight.camera.getProjectionMatrix() * mLight.camera.getViewMatrix() );
		mShadowShader->uniform( "uShadowTechnique", mShadowTechnique );
		mShadowShader->uniform( "uDepthBias", mDepthBias );
		mShadowShader->uniform( "uOnlyShadowmap", mOnlyShadowmap );
		mShadowShader->uniform( "uRandomOffset", mRandomOffset );
		mShadowShader->uniform( "uNumRandomSamples", mNumRandomSamples );
		mShadowShader->uniform( "uEnableNormSlopeOffset", mEnableNormSlopeOffset );
		mShadowShader->uniform( "uLightPos", vec3( gl::getModelView() * vec4( mLight.viewpoint, 1.0 ) ) );
		
		drawScene( spinAngle, mShadowShader );
	}
	gl::disable( GL_POLYGON_OFFSET_FILL );
	
	// Render light direction vector
	gl::drawVector( mLight.viewpoint, 4.5f * normalize( mLight.viewpoint ) );
}

void Application::keyDown( KeyEvent event )
{
	switch (event.getChar())
	{
	case 'w':
		
		break;
	default:
		break;
	}
	if( event.getChar() == 'f' ) {
		app::setFullScreen( !app::isFullScreen() );
	}
}

void prepareSettings( App::Settings *settings )
{
#if ! defined( CINDER_GL_ES )
	//settings->enableHighDensityDisplay();
	settings->setWindowSize( 900, 900 );
#endif
}

CINDER_APP( Application, RendererGl( RendererGl::Options().msaa( 16 ) ), prepareSettings )
