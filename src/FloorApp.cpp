#include "cinder/app/AppBasic.h"
#include "cinder/ImageIO.h"
#include "cinder/gl/Texture.h"
#include "cinder/MayaCamUI.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Perlin.h"
#include "cinder/params/Params.h"
#include "Resources.h"
#include "Particle.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 800

#define TERRAIN_SIZE 100
#define TERRAIN_SPACING 0.7f

#define PARTICLE_COUNT 100

using namespace ci;
using namespace ci::app;
using namespace std;

class FloorApp : public AppBasic {
  public:
    void prepareSettings( Settings *settings );
	void setup();
	void mouseDown( MouseEvent event );	
    void mouseDrag( MouseEvent event );
    void resize( ResizeEvent event );
	void update();
	void draw();
    
    gl::GlslProg loadShader( const char* vertexPath, const char* fragmentPath );
    void renderScene();
    void addVertex( vector<uint32_t>* indices, vector<Vec2f>* texCoords, vector<Vec3f>* positions, int x, int z, int i );
    float getHeightForTexCoord( Vec2f texCoord );
    
    MayaCamUI mMayaCam;
    
    Vec3f mLightDir;
    
    params::InterfaceGl mParams;
    
    Perlin mPerlin;
    
    vector<Particle> mParticles;
    
    float mDistortPower;
    
    gl::Fbo mScreenFbo, mBufferFbo;
    gl::VboMesh mTerrainMesh;
    gl::GlslProg mFloorShader, mDistortShader;
    gl::Texture mNormalMap, mColorMap, mAOMap;
    Surface mHeightMap;
};

void FloorApp::prepareSettings( Settings *settings ){
    settings->setWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
}

void FloorApp::setup()
{
    mNormalMap = gl::Texture(loadImage( loadResource("cracked-rock-normal.png") ));
    mColorMap = gl::Texture(loadImage( loadResource("cracked-rock-diffuse.png") ));
    mAOMap = gl::Texture(loadImage( loadResource("cracked-rock-ao.png") ));
    mHeightMap = Surface(loadImage( loadResource("cracked-rock-height.png") ));
    
    mNormalMap.setWrap( GL_REPEAT, GL_REPEAT );
    mColorMap.setWrap( GL_REPEAT, GL_REPEAT );
    mAOMap.setWrap( GL_REPEAT, GL_REPEAT );
    
//    try {
//        mFloorShader = gl::GlslProg( loadResource( RES_FLOOR_VERTEX ), loadResource( RES_FLOOR_FRAGMENT ) );
//    }
//    catch ( gl::GlslProgCompileExc &exc ) {
//        cout << "shader error: " << std::endl;
//        cout << exc.what();
//    }
    
    mFloorShader = loadShader("floor.glsl", "floor-frag.glsl");
    mDistortShader = loadShader("floor.glsl", "distort-frag.glsl");
    
    mLightDir = Vec3f(-0.25f, 0.25f, 1.0f);
    mLightDir.normalize();
    
    mDistortPower = 0.02f;
    
    mParams = params::InterfaceGl("Params", Vec2i(300,150));
    mParams.addParam("Light Direction", &mLightDir );
    mParams.addParam("Distort Power", &mDistortPower, "min=0.0 max=0.4 step=0.001" );
    
    
    gl::Fbo::Format aaFormat;
    aaFormat.setSamples(4);
    mScreenFbo = gl::Fbo( WINDOW_WIDTH, WINDOW_HEIGHT, aaFormat );
    
    gl::Fbo::Format format;
    mBufferFbo = gl::Fbo( WINDOW_WIDTH, WINDOW_HEIGHT, format );
    
    mPerlin = Perlin();
    
    int totalVertices = TERRAIN_SIZE * TERRAIN_SIZE;
//    int totalQuads = (TERRAIN_SIZE-1) * (TERRAIN_SIZE-1);
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();
    layout.setStaticNormals();
    layout.setStaticTexCoords2d();
    mTerrainMesh = gl::VboMesh( totalVertices*6, totalVertices * 6, layout, GL_TRIANGLES );

    vector<uint32_t> indices;
    vector<Vec2f> texCoords;
    vector<Vec3f> positions;
    int i = 0;
    for( int x = 0; x < TERRAIN_SIZE-1; ++x ){
        for( int z = 0; z < TERRAIN_SIZE-1; ++z ){
            addVertex( &indices, &texCoords, &positions, x, z, i );
            addVertex( &indices, &texCoords, &positions, x + 1, z, i + 1 );
            addVertex( &indices, &texCoords, &positions, x, z + 1, i + 2 );
            
            addVertex( &indices, &texCoords, &positions, x, z + 1, i + 3 );
            addVertex( &indices, &texCoords, &positions, x + 1, z, i + 4 );
            addVertex( &indices, &texCoords, &positions, x + 1, z + 1, i + 5 );
            i += 6;
            
        }
    }
    mTerrainMesh.bufferIndices( indices );
    mTerrainMesh.bufferTexCoords2d( 0, texCoords );
    mTerrainMesh.bufferPositions( positions );
//    mTerrainMesh.enableClientStates();
    
    
    // set up the camera
	CameraPersp cam;
	cam.setEyePoint( Vec3f(5.0f, 10.0f, 40.0f) );
	cam.setCenterOfInterestPoint( Vec3f(0.0f, 2.5f, 0.0f) );
	cam.setPerspective( 60.0f, getWindowAspectRatio(), 1.0f, 1000.0f );
	mMayaCam.setCurrentCam( cam );
    
    for(int i=0;i<PARTICLE_COUNT;i++){
        Particle p = Particle();
        p.position = Vec3f(mPerlin.dfBm((float)i,(float)(100-i))) + Vec3f(0.0f, 10.0f, 0.0f);
        mParticles.push_back(p);
    }
}

void FloorApp::addVertex( vector<uint32_t>* indices, vector<Vec2f>* texCoords, vector<Vec3f>* positions, int x, int z, int i ){
    indices->push_back(i);
//    float height = mPerlin.fBm(((float)x)/10.0f, ((float)z)/10.0f) * 30.0f;
    Vec2f texCoord = Vec2f( ((float)x)/TERRAIN_SIZE, ((float)z)/TERRAIN_SIZE );
    float height = getHeightForTexCoord(texCoord);
//    cout << "height: " << height << endl;
                                    
    positions->push_back( Vec3f( x * TERRAIN_SPACING, height, z * TERRAIN_SPACING ) );
    texCoords->push_back( texCoord );
}

void FloorApp::update()
{
    gl::enable( GL_TEXTURE_2D );
    gl::enableDepthRead();
    gl::enableDepthWrite();
//    gl::enableWireframe();
    
    mScreenFbo.bindFramebuffer();
    
    renderScene();
    
    mScreenFbo.unbindFramebuffer();
}

void FloorApp::draw()
{
    
    mBufferFbo.bindFramebuffer();
    
    gl::clear( Color::black() );
    
    mScreenFbo.getTexture(0).bind(0);
    mNormalMap.bind(1);
    
    mDistortShader.bind();
    mDistortShader.uniform("texDiffuse", 0);
    mDistortShader.uniform("texNormal", 1);
    mDistortShader.uniform("distortPower", mDistortPower );
    
//    gl::draw( mScreenFbo.getTexture(0), getWindowBounds() );
    gl::drawSolidRect( getWindowBounds() );
    
    mScreenFbo.getTexture(0).unbind();
    mNormalMap.unbind();
    mDistortShader.unbind();
    
    mBufferFbo.unbindFramebuffer();
    
    gl::clear( Color( 0, 0, 0 ) );
    gl::draw( mBufferFbo.getTexture(0), getWindowBounds() );
    
    
    
//    renderScene();
    
    params::InterfaceGl::draw();
	
}

void FloorApp::renderScene(){
    // clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
    
    // set up the camera 
	gl::pushMatrices();
	gl::setMatrices( mMayaCam.getCamera() );
    
    mColorMap.bind(0);
    mNormalMap.bind(1);
    mAOMap.bind(2);
    
    mFloorShader.bind();
    mFloorShader.uniform("texDiffuse", 0);
    mFloorShader.uniform("texNormal", 1);
    mFloorShader.uniform("texAO", 2);
    mFloorShader.uniform("lightDir", mLightDir);
    
//    for(int i=0;i<8;i++){
//        gl::drawCube( Vec3f(0.0f, i*10.0f, 0.0f), Vec3f(30.0f, 5.0f, 30.0f) );
//    }
//    gl::drawSphere( Vec3f::zero(), 4.0f, 40 );
    
    gl::translate( Vec3f(-TERRAIN_SIZE*TERRAIN_SPACING*0.5f,0.0f, -TERRAIN_SIZE*TERRAIN_SPACING*0.5f) );
    
    for(int x=-2; x<2; ++x){
        for(int z=-2; z<2; ++z){
            gl::pushMatrices();
            gl::translate( Vec3f(x*(TERRAIN_SIZE-1)*TERRAIN_SPACING,0.0f,z*(TERRAIN_SIZE-1)*TERRAIN_SPACING) );
            gl::draw( mTerrainMesh );
            gl::popMatrices();
        }
    }
    for(int i=0;i<PARTICLE_COUNT;i++){
        mParticles[i].update();
        mParticles[i].draw();
    }
    mFloorShader.unbind();
    mColorMap.unbind();
    mNormalMap.unbind();
    mAOMap.unbind();
    
    
    
    gl::popMatrices();
}

gl::GlslProg FloorApp::loadShader( const char* vertexPath, const char* fragmentPath ){
    gl::GlslProg program;
    try {
        program = gl::GlslProg( loadResource( vertexPath ), loadResource( fragmentPath ) );
    }
    catch ( gl::GlslProgCompileExc &exc ) {
        cout << "shader error: " << std::endl;
        cout << exc.what();
    }
    return program;
}

float FloorApp::getHeightForTexCoord( Vec2f texCoord ){
    Vec2i imageCoord = Vec2i( texCoord.x * mHeightMap.getWidth(), texCoord.y * mHeightMap.getHeight() );
    return mHeightMap.getPixel(imageCoord).r * (20.0f/255.0f);
}

void FloorApp::mouseDown( MouseEvent event )
{	
	// let the camera handle the interaction
	mMayaCam.mouseDown( event.getPos() );
}

void FloorApp::mouseDrag( MouseEvent event )
{    
	// let the camera handle the interaction
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void FloorApp::resize( ResizeEvent event )
{
	// adjust aspect ratio
	CameraPersp cam = mMayaCam.getCamera();
	cam.setAspectRatio( getWindowAspectRatio() );
	mMayaCam.setCurrentCam( cam );
}


CINDER_APP_BASIC( FloorApp, RendererGl )
