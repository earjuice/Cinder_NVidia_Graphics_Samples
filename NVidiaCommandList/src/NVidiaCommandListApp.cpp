//----------------------------------------------------------------------------------
// File:        gl4-maxwell\NvCommandList/basic-nvcommandlist.cpp
// SDK Version: v3.00 
// Email:       gameworks@nvidia.com
// Site:        http://developer.nvidia.com/
//
// Copyright (c) 2014-2015, NVIDIA CORPORATION. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//----------------------------------------------------------------------------------

#define DEBUG_FILTER     1

#define USE_PROGRAM_FILTER  1



#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/Utilities.h"
#include "cinder/Log.h"

//#include "geometry.hpp"
//#include "nvtoken.hpp"
//#include "common.h"
#include "headers.h"

#ifdef USE_IMGUI
#include "CinderImGui.h"
#else
#include "cinder/params/Params.h"
#endif //USE_IMGUI

#define NV_BUFFER_OFFSET(i) ((char *)NULL + (i))


using namespace ci;
using namespace ci::app;
using namespace std;
using namespace nvtoken;
using namespace basiccmdlist;


int const SAMPLE_SIZE_WIDTH(800);
int const SAMPLE_SIZE_HEIGHT(600);
int const SAMPLE_MAJOR_VERSION(4);
int const SAMPLE_MINOR_VERSION(3);

static const int numObjects = 1024;
static const int grid = 64;
static const float globalscale = 8.0f;




class NVidiaCommandListApp : public App {
  public:
	  NVidiaCommandListApp() : mode(0) {};
	  ~NVidiaCommandListApp() {};
	  void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
	void resize() override;

	void initRendering();
	GLuint createShader(gl::GlslProgRef glsl, gl::GlslProg::Format format);//, int32_t count);

	//void initUI();
//	void configurationCallback(NvGLConfiguration& config);
	GLuint compileProgram(ShaderSourceItem *src, int32_t count);
	//GLuint compileProgram(gl::GlslProgRef *glsl, int32_t count);


	GLSLProgram *m_drawScene, *m_drawSceneGeo;
	CameraPersp mCam;
	CameraUi mCamUI;

	private:
		gl::GlslProgRef mGlsl, mGlslGeo;
		gl::FboRef mFbo;
		GLuint getMainFBO() { return 0; }//mFbo->getId(); }
		string sources_scene_str[2], sources_sceneGeo_str[3];
		Tweak    tweak;
		uint32_t mode;

		std::vector<ObjectInfo> objects;

		SceneData sceneUbo;
		bool      bindlessVboUbo;
		bool      hwsupport;

		
		//bool begin();
		void think(double time);
		//void resize(int width, int height);

		//bool initProgram();
		bool initFramebuffers(int width, int height);
		bool initScene();

		bool  initCommandList();
		void  updateCommandListState();

		void  drawStandard();
		void  drawTokenBuffer();
		void  drawTokenList();
		void  drawTokenEmulation();

		
		float frand();

		size_t uboAligned(size_t size) {

			return ((size + 255) / 256) * 256;
		}

		//int stringInExtensionString(const char*, const char*);
		//int sysExtensionSupported(const char*);

	
		vec3 lightDir;

	
};


void NVidiaCommandListApp::setup()
{
	// Set up camera 
	mCam = CameraPersp(getWindowWidth(), getWindowHeight(), 45.0f, 0.01f, 100.0f);
	mCam.lookAt(vec3(-2.221f, 2.0f, 15.859f), vec3(0.0f, 1.0f, 0.0f));
	//mCamUI.setCamera(&mCam);
	mCamUI = CameraUi(&mCam, getWindow(), -1);

	initRendering();
	//static bool renderedScene = false;
	static bool renderedScene = false;

	initFramebuffers(getWindowWidth(), getWindowHeight());

	if (!renderedScene) {

		initScene();
		initCommandList();
		renderedScene = true;
	}
//	initFramebuffers(getWindowWidth(), getWindowHeight());

	//if (!renderedScene) {

	//	initScene();
	//	initCommandList();
	//	renderedScene = true;
	//}
}

void NVidiaCommandListApp::mouseDown( MouseEvent event )
{
}

void NVidiaCommandListApp::update()
{

	tweak.mode = (DrawMode)mode;
	think(getElapsedSeconds());

}

void NVidiaCommandListApp::draw()
{
	{
		GLuint width = getWindowWidth();
		GLuint height = getWindowHeight();
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbos.scene);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, getMainFBO());
		glBlitFramebuffer(0, 0, width, height,
			0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}
	//gl::ScopedViewport scV(mFbo->getSize());
	//gl::clear();

	//gl::clear( Color( 0, 0, 0 ) ); 
	//gl::draw(mFbo->getColorTexture());
}

float NVidiaCommandListApp::frand() {
	return (float(rand() % RAND_MAX) / float(RAND_MAX));
}

GLuint NVidiaCommandListApp::compileProgram(ShaderSourceItem *src, int32_t count)
{

	const GLchar *const ptr[] = { "/" };
	GLuint program = glCreateProgram();

	int32_t i;
	for (i = 0; i < count; i++) {
		GLuint shader = glCreateShader(src[i].type);
		//console() << src[i].src << std::endl;

		const char* sourceItems[2];
		int sourceCount = 0;
		sourceItems[sourceCount++] = (src[i].src);

		glShaderSource(shader, sourceCount, sourceItems, 0);
		glCompileShaderIncludeARB(shader, 1, ptr, NULL);


		glAttachShader(program, shader);

		// can be deleted since the program will keep a reference
		glDeleteShader(shader);
	}

	glLinkProgram(program);

	// check if program linked
	GLint success = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &success);

	if (!success)
	{
		console() << "Program Linked Unsuccessfully" << std::endl;

		GLint bufLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
		if (bufLength) {
			char* buf = new char[bufLength];
			if (buf) {
				glGetProgramInfoLog(program, bufLength, NULL, buf);
				console() << "Could not link program: " << std::endl;
				console() <<  toString(buf) << std::endl;
				console() << "end wr program: " << std::endl;

				delete[] buf;
			}
		}
		glDeleteProgram(program);
		program = 0;
	}
	else {
		console() << "Program Linked Successfully" << std::endl;
	}

	return program;
}

GLuint NVidiaCommandListApp::createShader(gl::GlslProgRef glsl, gl::GlslProg::Format format){//, int32_t count) {

	// Create our pixel and vertex shader
	auto loadGlslProg = [&](const gl::GlslProg::Format& format) -> gl::GlslProgRef
	{
		gl::GlslProgRef glslProg;
		try {
			glslProg = gl::GlslProg::create(format);
		}
		catch (const Exception& ex) {
			//	CI_LOG_EXCEPTION(names, ex);
			const string names = format.getVertexPath().string() + " + " + format.getFragmentPath().string();
			ci::app::console() << "CI_LOG_EXCEPTION" << names << &ex << std::endl;
			quit();
		}
		return glslProg;
	};
	glsl = loadGlslProg(format);
	return glsl->getHandle();
}
const char *getRawFileName(string filename) 
{
	return filename.c_str();
}
void NVidiaCommandListApp::initRendering()
{
	console() << "GL_RENDERER " << glGetString(GL_RENDERER) << std::endl;
	console() << "GL_VERSION " << glGetString(GL_VERSION) << std::endl;
	auto gl_version = gl::getVersion();
	if (gl_version.first != 4 && gl_version.second < 4) {
		console() << "opengl should be set above v4,4" << std::endl;
		exit(EXIT_FAILURE); 
	}
	
	//this combines two ints into a single int for gl::GlslProg::Format().version
	int array[] = { gl_version.first,gl_version.second };
	int vers_Int = 0;
	for (int i = 0; i < 2; i++) {
		vers_Int *= 10;
		vers_Int += array[i];
	}
	vers_Int *= 10;

	console() << "gl_version int: " << vers_Int << std::endl;

	//sprintf(integer_string, "%d", integer);
	// Check extensions; exit on failure
	//if (!gl::isExtensionAvailable("GL_NV_vertex_buffer_unified_memory")) return;
	//if (!gl::isExtensionAvailable("GL_NV_shader_buffer_load")) return;
	//if (!gl::isExtensionAvailable("GL_EXT_direct_state_access")) return;
	//if (!gl::isExtensionAvailable("GL_NV_bindless_texture")) return;
	//if (!gl::isExtensionAvailable("GL_ARB_bindless_texture"))
	if (!gl::isExtensionAvailable("GL_NV_bindless_texture")) { console() << "This sample requires ARB_bindless_texture" << std::endl;exit(EXIT_FAILURE); }
	if (!gl::isExtensionAvailable("GL_NV_vertex_buffer_unified_memory")) { console() << "GL_NV_vertex_buffer_unified_memory" << std::endl;exit(EXIT_FAILURE); }

	bindlessVboUbo = GL_NV_vertex_buffer_unified_memory;


	if (!glNamedStringARB)
	{
	//	CI_LOG_E("Fatal Error", "App requires support for glNamedStringARB");
		console() << "This sample requires glNamedStringARB" << std::endl;
		exit(EXIT_FAILURE);
	}

	//NvAssetLoaderAddSearchPath("gl4-maxwell/NvCommandList");

	char *registerInclude = (char*) "/common.h";

//	int32_t len;
	/* draw scene*/
	m_drawScene = new GLSLProgram;

	auto srcHeader = (GLchar*)loadString(loadAsset("shaders/common.h")).c_str();
	//auto srcHeader = tIn;
	glNamedStringARB(GL_SHADER_INCLUDE_ARB, strlen(registerInclude), registerInclude, (GLint)strlen(srcHeader), srcHeader);


	/*
	ShaderSourceItem sources_scene[2];
	sources_scene[0].type = GL_VERTEX_SHADER;
	sources_scene_str[0] = loadString(loadAsset("shaders/scene.vert.glsl"));
	sources_scene[0].src = sources_scene_str[0].c_str();
	sources_scene[1].type = GL_FRAGMENT_SHADER;
	sources_scene_str[1] = loadString(loadAsset("shaders/scene.vert.glsl"));
	sources_scene[1].src = sources_scene_str[1].c_str();
	m_drawScene->setProgram(compileProgram(sources_scene, 2));
	*/
	static gl::GlslProg::Format glfrmt;
	glfrmt.version(vers_Int);
	glfrmt.vertex(loadAsset("shaders/scene.vert.glsl"));
	glfrmt.fragment(loadAsset("shaders/scene.frag.glsl"));
	glfrmt.preprocess(true);
	m_drawScene->setProgram(createShader(mGlsl, glfrmt));
	/* draw scene_geo */
	m_drawSceneGeo = new GLSLProgram;

	/*
	ShaderSourceItem sources_scene_geo[3];
	sources_scene_geo[0].type = GL_VERTEX_SHADER;
	sources_scene_geo[0].src = loadString(loadAsset("shaders/scene.vert.glsl")).data();
	sources_scene_geo[1].type = GL_GEOMETRY_SHADER;
	sources_scene_geo[1].src = loadString(loadAsset("shaders/scene.geo.glsl")).data();
	sources_scene_geo[2].type = GL_FRAGMENT_SHADER;
	sources_scene_geo[2].src = loadString(loadAsset("shaders/scene.frag.glsl")).data();
	m_drawSceneGeo->setProgram(compileProgram(sources_scene_geo, 3));
	*/
	static gl::GlslProg::Format glfrmtGeo;
	glfrmtGeo.version(vers_Int);
	glfrmtGeo.vertex(loadAsset("shaders/scene.vert.glsl"));
	glfrmtGeo.geometry(loadAsset("shaders/scene.geo.glsl"));
	glfrmtGeo.fragment(loadAsset("shaders/scene.frag.glsl"));
	glfrmtGeo.preprocess(true);
	m_drawSceneGeo->setProgram(createShader(mGlslGeo, glfrmtGeo));


	programs.draw_scene = 0, programs.draw_scene_geo = 1;
	textures.color = 0, textures.scene_color = 0, textures.scene_depthstencil = 0;
	fbos.scene = 0;
	buffers.box_ibo = 0, buffers.box_vbo = 0, buffers.objects_ubo = 0, buffers.scene_ubo = 0,
		buffers.sphere_ibo = 0, buffers.sphere_vbo = 0;

	texturesADDR.color = 0, texturesADDR.scene_color = 0, texturesADDR.scene_depthstencil = 0;
	buffersADDR.box_ibo = 0, buffersADDR.box_vbo = 0, buffersADDR.objects_ubo = 0, buffersADDR.scene_ubo = 0,
		buffersADDR.sphere_ibo = 0, buffersADDR.sphere_vbo = 0;

	cmdlist.state.programIncarnation++;

	GLuint defaultVAO;
	glGenVertexArrays(1, &defaultVAO);
	glBindVertexArray(defaultVAO);

	sceneUbo.shrinkFactor = 0.5f;
	lightDir = glm::normalize(vec3(-1, 1, 1));
	console() << "initRendering complete" << std::endl;

}
/*
void NVidiaCommandListApp::initUI() {

	if (mTweakBar) {

		NvTweakEnum<uint32_t> enumVals[] = {
			{ "standard", DRAW_STANDARD },
			{ "nvcmdlist emulated", DRAW_TOKEN_EMULATED },
			{ "nvcmdlist buffer", DRAW_TOKEN_BUFFER },
			{ "nvcmdlist list", DRAW_TOKEN_LIST },
	};

		mTweakBar->addPadding();
		mTweakBar->addEnum("Draw Mode:", mode, enumVals, TWEAKENUM_ARRAYSIZE(enumVals));
		mTweakBar->addValue("Shrink:", sceneUbo.shrinkFactor, 0.0f, 1.0f);

		mTweakBar->addPadding(5.0f);
		mTweakBar->addValue("LightDir_X:", lightDir.x, -1.0f, 1.0f);
		mTweakBar->addValue("LightDir_Y:", lightDir.y, -1.0f, 1.0f);
		mTweakBar->addValue("LightDir_Z:", lightDir.z, -1.0f, 1.0f);
		mTweakBar->syncValues();
}
	}

void NVidiaCommandListApp::configurationCallback(NvGLConfiguration& config) {

	config.depthBits = 24;
	config.stencilBits = 0;
	config.apiVer = NvGLAPIVersionGL4_4();
}
*/
bool NVidiaCommandListApp::initFramebuffers(int width, int height)
{
	console() << "initFramebuffers" << std::endl;
	mFbo = gl::Fbo::create(width, height, gl::Fbo::Format().depthBuffer());

//#if defined(ANDROID)
//	if (textures.scene_color && glIsSupportedREGAL("GL_ARB_bindless_texture"))
//#elif defined(LINUX) || defined(_WIN32)
	if (textures.scene_color && GL_ARB_bindless_texture)
//#endif
	{
		glMakeTextureHandleNonResidentARB(texturesADDR.scene_color);
		glMakeTextureHandleNonResidentARB(texturesADDR.scene_depthstencil);

	}

	if (textures.scene_color) glDeleteTextures(1, &textures.scene_color);
	glGenTextures(1, &textures.scene_color);

	glBindTexture(GL_TEXTURE_2D, textures.scene_color);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);

	if (textures.scene_depthstencil) glDeleteTextures(1, &textures.scene_depthstencil);
	glGenTextures(1, &textures.scene_depthstencil);

	glBindTexture(GL_TEXTURE_2D, textures.scene_depthstencil);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, width, height);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (fbos.scene) glDeleteFramebuffers(1, &fbos.scene);
	glGenFramebuffers(1, &fbos.scene);

	glBindFramebuffer(GL_FRAMEBUFFER, fbos.scene);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures.scene_color, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, textures.scene_depthstencil, 0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, getMainFBO());

	//mFbo->bindFramebuffer(mFbo->getId());
	//console() << "initFramebuffers mid" << std::endl;

	// COMMANDLIST
	// As stateobjects and cmdlist objects make references to texture memory, you need ensure residency,
	// as well as rebuilding state objects as FBO memory changes.

//#if defined(ANDROID)
//	if (glIsSupportedREGAL("GL_ARB_bindless_texture"))
//#elif defined(LINUX) || defined(_WIN32)
	if (GL_ARB_bindless_texture)
//#endif
	{
		texturesADDR.scene_color = glGetTextureHandleARB(textures.scene_color);
		texturesADDR.scene_depthstencil = glGetTextureHandleARB(textures.scene_depthstencil);
		glMakeTextureHandleResidentARB(texturesADDR.scene_color);
		glMakeTextureHandleResidentARB(texturesADDR.scene_depthstencil);
	}

	cmdlist.state.fboIncarnation++;
	
	console() << "mFbo: " << getMainFBO() << " fbos.scene: " << fbos.scene << std::endl;

	return true;
}

bool NVidiaCommandListApp::initScene()
{
	console() << "initScene" << std::endl;

	srand(1238);

	{
		// pattern texture
		int size = 32;
		std::vector<ivec4 >  texels;
		texels.resize(size * size);

		for (int y = 0; y < size; y++) {
			for (int x = 0; x < size; x++) {
				int pos = x + y * size;
				ivec4 texel;

				texel[0] = ((x + y ^ 127) & 15) * 17;
				texel[1] = ((x + y ^ 127) & 31) * 8;
				texel[2] = ((x + y ^ 127) & 63) * 4;
				texel[3] = 255;

				texels[pos] = texel;
			}
		}

		int mipMapLevels = 0;
		while (size) {
			mipMapLevels++;
			size /= 2;
		}
		size = 32;

		if (textures.color) glDeleteTextures(1, &textures.color);
		glGenTextures(1, &textures.color);

		glBindTexture(GL_TEXTURE_2D, textures.color);
		glTexStorage2D(GL_TEXTURE_2D, mipMapLevels, GL_RGBA8, size, size);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size, size, GL_RGBA, GL_UNSIGNED_BYTE, &texels[0]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		// this sample requires use of bindless texture
		texturesADDR.color = glGetTextureHandleARB(textures.color);
		glMakeTextureHandleResidentARB(texturesADDR.color);
	}

	{ // Scene Geometry

		geometry::Box<Vertex>     box;

		if (buffers.box_ibo) glDeleteBuffers(1, &buffers.box_ibo);
		glGenBuffers(1, &buffers.box_ibo);
		glNamedBufferStorageEXT(buffers.box_ibo, box.getTriangleIndicesSize(), &box.m_indicesTriangles[0], 0);

		if (buffers.box_vbo) glDeleteBuffers(1, &buffers.box_vbo);
		glGenBuffers(1, &buffers.box_vbo);
		glNamedBufferStorageEXT(buffers.box_vbo, box.getVerticesSize(), &box.m_vertices[0], 0);

		if (bindlessVboUbo) {
			glGetNamedBufferParameterui64vNV(buffers.box_ibo, GL_BUFFER_GPU_ADDRESS_NV, &buffersADDR.box_ibo);
			glGetNamedBufferParameterui64vNV(buffers.box_vbo, GL_BUFFER_GPU_ADDRESS_NV, &buffersADDR.box_vbo);
			glMakeNamedBufferResidentNV(buffers.box_ibo, GL_READ_ONLY);
			glMakeNamedBufferResidentNV(buffers.box_vbo, GL_READ_ONLY);
		}

		geometry::Sphere<Vertex>  sphere;

		if (buffers.sphere_ibo) glDeleteBuffers(1, &buffers.sphere_ibo);
		glGenBuffers(1, &buffers.sphere_ibo);
		glNamedBufferStorageEXT(buffers.sphere_ibo, sphere.getTriangleIndicesSize(), &sphere.m_indicesTriangles[0], 0);

		if (buffers.sphere_vbo) glDeleteBuffers(1, &buffers.sphere_vbo);
		glGenBuffers(1, &buffers.sphere_vbo);
		glNamedBufferStorageEXT(buffers.sphere_vbo, sphere.getVerticesSize(), &sphere.m_vertices[0], 0);

		if (bindlessVboUbo) {
			glGetNamedBufferParameterui64vNV(buffers.sphere_ibo, GL_BUFFER_GPU_ADDRESS_NV, &buffersADDR.sphere_ibo);
			glGetNamedBufferParameterui64vNV(buffers.sphere_vbo, GL_BUFFER_GPU_ADDRESS_NV, &buffersADDR.sphere_vbo);
			glMakeNamedBufferResidentNV(buffers.sphere_ibo, GL_READ_ONLY);
			glMakeNamedBufferResidentNV(buffers.sphere_vbo, GL_READ_ONLY);
		}

		// Scene objects
		if (buffers.objects_ubo) glDeleteBuffers(1, &buffers.objects_ubo);
		glGenBuffers(1, &buffers.objects_ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, buffers.objects_ubo);
		glBufferData(GL_UNIFORM_BUFFER, uboAligned(sizeof(ObjectData)) * numObjects, NULL, GL_STATIC_DRAW);
		if (bindlessVboUbo) {
			glGetNamedBufferParameterui64vNV(buffers.objects_ubo, GL_BUFFER_GPU_ADDRESS_NV, &buffersADDR.objects_ubo);
			glMakeNamedBufferResidentNV(buffers.objects_ubo, GL_READ_ONLY);
		}

		objects.reserve(numObjects);
		for (int i = 0; i < numObjects; i++) {
			ObjectData  ubodata;

			vec3  pos(frand() * float(grid),
				frand() * float(grid),
				frand() * float(grid / 2));

			float scale = globalscale / float(grid);
			scale += (frand()) * 0.25f;

			pos -= vec3(grid / 2, grid / 2, grid / 4);
			pos /= float(grid) / globalscale;

			float angle = (frand()) * 180.f;

			glm::mat4 translation_mat = glm::translate(translation_mat, vec3(pos.x, pos.y, pos.z) );// nv::translation(translation_mat, pos.x, pos.y, pos.z);
			glm::mat4 scale_mat(1.);
			//scale_mat.make_identity();
			
			glm::scale(scale_mat, vec3(scale));
			//scale_mat.set_scale(scale);
			glm::mat4 rotationX_mat = glm::rotate((mat3)rotationX_mat, angle);
			//glm::mat4 rotationX_mat = mat4(1.f)* glm::rotateX;// nv::rotationX(rotationX_mat, angle);

			ubodata.worldMatrix = translation_mat * scale_mat * rotationX_mat;
			ubodata.worldMatrixIT = glm::transpose(glm::inverse(ubodata.worldMatrix));
			ubodata.texScale.x = rand() % 2 + 1.0f;
			ubodata.texScale.y = rand() % 2 + 1.0f;
			ubodata.color = glm::vec4(frand(), frand(), frand(), 1.0f);

			ubodata.texColor = texturesADDR.color; // bindless texture used

			glBufferSubData(GL_UNIFORM_BUFFER, uboAligned(sizeof(ObjectData)) * i, sizeof(ObjectData), &ubodata);

			ObjectInfo  info;
			info.program = pos.x < 0 ? programs.draw_scene_geo : programs.draw_scene;

			if (rand() % 2) {
				info.ibo = buffers.sphere_ibo;
				info.vbo = buffers.sphere_vbo;
				info.iboADDR = buffersADDR.sphere_ibo;
				info.vboADDR = buffersADDR.sphere_vbo;
				info.numIndices = sphere.getTriangleIndicesCount();
			}
			else {
				info.ibo = buffers.box_ibo;
				info.vbo = buffers.box_vbo;
				info.iboADDR = buffersADDR.box_ibo;
				info.vboADDR = buffersADDR.box_vbo;
				info.numIndices = box.getTriangleIndicesCount();
			}

			objects.push_back(info);
		}

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	{ // Scene UBO
		if (buffers.scene_ubo) glDeleteBuffers(1, &buffers.scene_ubo);
		glGenBuffers(1, &buffers.scene_ubo);

		glBindBuffer(GL_UNIFORM_BUFFER, buffers.scene_ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneData), NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		if (bindlessVboUbo) {
			glGetNamedBufferParameterui64vNV(buffers.scene_ubo, GL_BUFFER_GPU_ADDRESS_NV, &buffersADDR.scene_ubo);
			glMakeNamedBufferResidentNV(buffers.scene_ubo, GL_READ_ONLY);
		}
	}
	console() << "initScene complete" << std::endl;

	return true;
}

typedef void(*NVPproc)(void);
NVPproc sysGetProcAddress(const char* name) {

	NVPproc proc = NULL;
//#if defined(LINUX) || defined (_WIN32)
	proc = (NVPproc)wglGetProcAddress(name);
//#elif defined(ANDROID)
//	proc = (NVPproc)eglGetProcAddress(name);
//#endif

	return proc;
}

bool NVidiaCommandListApp::initCommandList()
{
	hwsupport = init_NV_command_list(sysGetProcAddress) ? true : false;

	nvtokenInitInternals(hwsupport, bindlessVboUbo);
	cmdlist.statesystem.init();

	{
		cmdlist.statesystem.generate(1, &cmdlist.stateid_draw);
		cmdlist.statesystem.generate(1, &cmdlist.stateid_draw_geo);
	}
	if (hwsupport) {
		glCreateStatesNV(1, &cmdlist.stateobj_draw);
		glCreateStatesNV(1, &cmdlist.stateobj_draw_geo);

		glGenBuffers(1, &cmdlist.tokenBuffer);
		glCreateCommandListsNV(1, &cmdlist.tokenCmdList);
	}
	else {
		cmdlist.stateobj_draw = 1;
		cmdlist.stateobj_draw_geo = 2;
	}

	// create actual token stream from our scene
	{
		NVTokenSequence& seq = cmdlist.tokenSequence;
		std::string& stream = cmdlist.tokenData;

		size_t offset = 0;

		// at first we bind the scene ubo to all used stages
		{
			NVTokenUbo  ubo;
			ubo.setBuffer(buffers.scene_ubo, buffersADDR.scene_ubo, 0, sizeof(SceneData));
			ubo.setBinding(UBO_SCENE, NVTOKEN_STAGE_VERTEX);
			nvtokenEnqueue(stream, ubo);
			ubo.setBinding(UBO_SCENE, NVTOKEN_STAGE_GEOMETRY);
			nvtokenEnqueue(stream, ubo);
			ubo.setBinding(UBO_SCENE, NVTOKEN_STAGE_FRAGMENT);
			nvtokenEnqueue(stream, ubo);
		}

		// then we iterate over all objects in our scene
		GLuint lastStateobj = 0;
		for (size_t i = 0; i < objects.size(); i++) {
			const ObjectInfo& obj = objects[i];

			GLuint usedStateobj = obj.program == programs.draw_scene ? cmdlist.stateobj_draw : cmdlist.stateobj_draw_geo;

			bool cond = lastStateobj != 0 && (usedStateobj != lastStateobj || !USE_PROGRAM_FILTER);
			if (cond) {
				// Whenever our program changes a new stateobject is required,
				// hence the current sequence gets appended
				seq.offsets.push_back(offset);
				seq.sizes.push_back(GLsizei(stream.size() - offset));
				seq.states.push_back(lastStateobj);

				// By passing the fbo here, it means we can render objects
				// even as the fbos get resized (and their textures changed).
				// If we would pass 0 it would mean the stateobject's fbo was used
				// which means on fbo resizes we would have to recreate all stateobjects.
				seq.fbos.push_back(fbos.scene);


				// new sequence start
				offset = stream.size();
			}

			NVTokenVbo vbo;
			vbo.setBinding(0);
			vbo.setBuffer(obj.vbo, obj.vboADDR, 0);
			nvtokenEnqueue(stream, vbo);

			NVTokenIbo ibo;
			ibo.setType(GL_UNSIGNED_INT);
			ibo.setBuffer(obj.ibo, obj.iboADDR);
			nvtokenEnqueue(stream, ibo);

			NVTokenUbo ubo;
			ubo.setBuffer(buffers.objects_ubo, buffersADDR.objects_ubo, GLuint(uboAligned(sizeof(ObjectData))*i), sizeof(ObjectData));
			ubo.setBinding(UBO_OBJECT, NVTOKEN_STAGE_VERTEX);
			nvtokenEnqueue(stream, ubo);
			ubo.setBinding(UBO_OBJECT, NVTOKEN_STAGE_FRAGMENT);
			nvtokenEnqueue(stream, ubo);

			if (usedStateobj == cmdlist.stateobj_draw_geo) {
				// also add for geometry stage
				ubo.setBinding(UBO_OBJECT, NVTOKEN_STAGE_GEOMETRY);
				nvtokenEnqueue(stream, ubo);
			}

			NVTokenDrawElems  draw;
			draw.setParams(obj.numIndices);
			// be aware the stateobject's primitive mode must be compatible!
			draw.setMode(GL_TRIANGLES);
			nvtokenEnqueue(stream, draw);

			lastStateobj = usedStateobj;
		}

		seq.offsets.push_back(offset);
		seq.sizes.push_back(GLsizei(stream.size() - offset));
		seq.fbos.push_back(fbos.scene);
		seq.states.push_back(lastStateobj);
	}

	if (hwsupport) {
		// upload the tokens once, so we can reuse them efficiently
		glNamedBufferStorageEXT(cmdlist.tokenBuffer, cmdlist.tokenData.size(), &cmdlist.tokenData[0], 0);

		// for list generation convert offsets to pointers
		cmdlist.tokenSequenceList = cmdlist.tokenSequence;
		for (size_t i = 0; i < cmdlist.tokenSequenceList.offsets.size(); i++) {
			cmdlist.tokenSequenceList.offsets[i] += (GLintptr)&cmdlist.tokenData[0];
		}
	}

	{
		// for emulation we have to convert the stateobject ids to statesystem ids
		cmdlist.tokenSequenceEmu = cmdlist.tokenSequence;
		for (size_t i = 0; i < cmdlist.tokenSequenceEmu.states.size(); i++) {
			GLuint oldstate = cmdlist.tokenSequenceEmu.states[i];
			cmdlist.tokenSequenceEmu.states[i] =
				(oldstate == cmdlist.stateobj_draw) ? cmdlist.stateid_draw : cmdlist.stateid_draw_geo;
		}
	}

	updateCommandListState();

	return true;
}

void NVidiaCommandListApp::updateCommandListState()
{

	if (cmdlist.state.programIncarnation != cmdlist.captured.programIncarnation)
	{
		// generic state shared by both programs
		glBindFramebuffer(GL_FRAMEBUFFER, fbos.scene);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		glEnableVertexAttribArray(VERTEX_POS);
		glEnableVertexAttribArray(VERTEX_NORMAL);
		glEnableVertexAttribArray(VERTEX_UV);

		glVertexAttribFormat(VERTEX_POS, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
		glVertexAttribFormat(VERTEX_NORMAL, 3, GL_SHORT, GL_TRUE, offsetof(Vertex, normal));
		glVertexAttribFormat(VERTEX_UV, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
		glVertexAttribBinding(VERTEX_POS, 0);
		glVertexAttribBinding(VERTEX_NORMAL, 0);
		glVertexAttribBinding(VERTEX_UV, 0);
		// prime the stride parameter, used by bindless VBO and statesystem
		glBindVertexBuffer(0, 0, 0, sizeof(Vertex));

		// temp workaround
		if (hwsupport) {
			glBufferAddressRangeNV(GL_VERTEX_ATTRIB_ARRAY_ADDRESS_NV, 0, 0, 0);
			glBufferAddressRangeNV(GL_ELEMENT_ARRAY_ADDRESS_NV, 0, 0, 0);
			glBufferAddressRangeNV(GL_UNIFORM_BUFFER_ADDRESS_NV, UBO_OBJECT, 0, 0);
			glBufferAddressRangeNV(GL_UNIFORM_BUFFER_ADDRESS_NV, UBO_SCENE, 0, 0);
		}

		// let's create the first stateobject
		glUseProgram(m_drawScene->getProgram());

		if (hwsupport) {
			glStateCaptureNV(cmdlist.stateobj_draw, GL_TRIANGLES);
		}

		StateSystem::State state;
		state.getGL(); // this is a costly operation
		cmdlist.statesystem.set(cmdlist.stateid_draw, state, GL_TRIANGLES);


		// The statesystem also provides an alternative approach
		// to getGL, by manipulating the state data directly.
		// When no getGL is called the state data matches the default
		// state of OpenGL.

		state.program.program = m_drawSceneGeo->getProgram();
		cmdlist.statesystem.set(cmdlist.stateid_draw_geo, state, GL_TRIANGLES);
		if (hwsupport) {
			// we can apply the state directly with
			// glUseProgram( state.program.program );
			// or
			// state.applyGL(false,false, 1<<StateSystem::DYNAMIC_VIEWPORT);
			// or more efficiently using the system which will use state diffs
			cmdlist.statesystem.applyGL(cmdlist.stateid_draw_geo, cmdlist.stateid_draw, true);

			glStateCaptureNV(cmdlist.stateobj_draw_geo, GL_TRIANGLES);
		}

		// since we will toggle between two states, let the emulation cache the differences
		cmdlist.statesystem.prepareTransition(cmdlist.stateid_draw, cmdlist.stateid_draw_geo);
		cmdlist.statesystem.prepareTransition(cmdlist.stateid_draw_geo, cmdlist.stateid_draw);

		glDisableVertexAttribArray(VERTEX_POS);
		glDisableVertexAttribArray(VERTEX_NORMAL);
		glDisableVertexAttribArray(VERTEX_UV);
		glBindFramebuffer(GL_FRAMEBUFFER, getMainFBO());
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
	}

	if (hwsupport && (
		cmdlist.state.programIncarnation != cmdlist.captured.programIncarnation ||
		cmdlist.state.fboIncarnation != cmdlist.captured.fboIncarnation))
	{
		// Because the commandlist object takes all state information 
		// from the objects during compile, we have to update commandlist
		// every time a state object or fbo changes.
		NVTokenSequence &seq = cmdlist.tokenSequenceList;
		glCommandListSegmentsNV(cmdlist.tokenCmdList, 1);
		glListDrawCommandsStatesClientNV(cmdlist.tokenCmdList, 0, (const void**)&seq.offsets[0], &seq.sizes[0], &seq.states[0], &seq.fbos[0], int(seq.states.size()));
		glCompileCommandListNV(cmdlist.tokenCmdList);
	}

	cmdlist.captured = cmdlist.state;
}

void NVidiaCommandListApp::think(double time)
{
	int width = getWindowWidth();
	int height = getWindowHeight();
	//gl::ScopedViewport scVp(vec2(width, height));

	{
		sceneUbo.viewport = ivec2(width, height);



		glm::mat4 projection = mCam.getProjectionMatrix();//glm::perspective( 45.f * nv_pi / 180.0f, float(width) / float(height), 0.1f, 1000.0f);//
		
		vec3 sceneOrbit = vec3(0.0f);
		float sceneDimension = float(grid) * 0.2f;
		glm::mat4 view = mCam.getViewMatrix();//glm::lookAt( sceneOrbit - vec3(0, 0, -sceneDimension), sceneOrbit, vec3(0, 1, 0));//
		
		sceneUbo.viewProjMatrix = projection * view;
		sceneUbo.viewProjMatrixI = glm::inverse(sceneUbo.viewProjMatrix);
		sceneUbo.viewMatrix = view;
		sceneUbo.viewMatrixI = glm::inverse(view);
		sceneUbo.viewMatrixIT = glm::transpose(sceneUbo.viewMatrixI);

		sceneUbo.wLightPos = vec4(lightDir * float(grid), 1.0f);
		sceneUbo.time = float(time);

		glNamedBufferSubDataEXT(buffers.scene_ubo, 0, sizeof(SceneData), &sceneUbo);

		glBindFramebuffer(GL_FRAMEBUFFER, fbos.scene);
		glViewport(0, 0, width, height);
		//gl::ScopedFramebuffer (vec2(width, height));
		//gl::ScopedViewport scVp(vec2(width,height));
	//	gl::clear();
		vec4   bgColor(1., 0.2, 0.2, 0.0);
		glClearColor(bgColor.x, bgColor.y, bgColor.z, bgColor.w);
		glClearDepthf(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	{
		switch (tweak.mode) {
		case DRAW_STANDARD:
			drawStandard();
			break;
		case DRAW_TOKEN_EMULATED:
			drawTokenEmulation();
			break;
		case DRAW_TOKEN_BUFFER:
			drawTokenBuffer();
			break;
		case DRAW_TOKEN_LIST:
			drawTokenList();
			break;
		}
	}


	//console() << "mFbo: " << getMainFBO() << " fbos.scene: " << fbos.scene << std::endl;
}
void NVidiaCommandListApp::resize() {
	
}
void NVidiaCommandListApp::drawStandard()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glVertexAttribFormat(VERTEX_POS, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
	glVertexAttribFormat(VERTEX_NORMAL, 3, GL_SHORT, GL_TRUE, offsetof(Vertex, normal));
	glVertexAttribFormat(VERTEX_UV, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
	glVertexAttribBinding(VERTEX_POS, 0);
	glVertexAttribBinding(VERTEX_NORMAL, 0);
	glVertexAttribBinding(VERTEX_UV, 0);

	glEnableVertexAttribArray(VERTEX_POS);
	glEnableVertexAttribArray(VERTEX_NORMAL);
	glEnableVertexAttribArray(VERTEX_UV);

	glBindBufferBase(GL_UNIFORM_BUFFER, UBO_SCENE, buffers.scene_ubo);

	GLuint lastProg = 0;
	for (int i = 0; i < (int)objects.size(); i++) {
		const ObjectInfo&  obj = objects[i];
		GLuint usedProg = 0;

		if (programs.draw_scene_geo == obj.program)
			usedProg = m_drawSceneGeo->getProgram();
		else if (programs.draw_scene == obj.program)
			usedProg = m_drawScene->getProgram();

		bool cond = usedProg != lastProg || !USE_PROGRAM_FILTER;
		if (cond) {
			// simple redundancy tracker
			glUseProgram(usedProg);
			lastProg = usedProg;
		}

		glBindBufferRange(GL_UNIFORM_BUFFER, UBO_OBJECT, buffers.objects_ubo, uboAligned(sizeof(ObjectData))*i, sizeof(ObjectData));

		glBindVertexBuffer(0, obj.vbo, 0, sizeof(Vertex));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.ibo);
		glDrawElements(GL_TRIANGLES, obj.numIndices, GL_UNSIGNED_INT, NV_BUFFER_OFFSET(0));
	}

	glDisableVertexAttribArray(VERTEX_POS);
	glDisableVertexAttribArray(VERTEX_NORMAL);
	glDisableVertexAttribArray(VERTEX_UV);

	glBindBufferBase(GL_UNIFORM_BUFFER, UBO_SCENE, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, UBO_OBJECT, 0);
	glBindVertexBuffer(0, 0, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void NVidiaCommandListApp::drawTokenBuffer()
{
	if (cmdlist.state != cmdlist.captured) {
		//updateCommandListState();
	}

	glDrawCommandsStatesNV(cmdlist.tokenBuffer,
		&cmdlist.tokenSequence.offsets[0],
		&cmdlist.tokenSequence.sizes[0],
		&cmdlist.tokenSequence.states[0],
		&cmdlist.tokenSequence.fbos[0],
		GLuint(cmdlist.tokenSequence.offsets.size()));
}

void NVidiaCommandListApp::drawTokenList()
{
	if (cmdlist.state != cmdlist.captured) {
		updateCommandListState();
	}

	glCallCommandListNV(cmdlist.tokenCmdList);
}

void NVidiaCommandListApp::drawTokenEmulation()
{
	if (bindlessVboUbo) {
		glEnableClientState(GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV);
		glEnableClientState(GL_ELEMENT_ARRAY_UNIFIED_NV);
		glEnableClientState(GL_UNIFORM_BUFFER_UNIFIED_NV);
#if _DEBUG
		// we are using what is considered "unsafe" addresses which will throw tons of warnings
		GLuint msgid = 65537;
		glDebugMessageControlARB(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 1, &msgid, GL_FALSE);
#endif
	}

	if (cmdlist.state != cmdlist.captured) {
		updateCommandListState();
	}

	nvtokenDrawCommandsStatesSW(&cmdlist.tokenData[0], cmdlist.tokenData.size(),
		&cmdlist.tokenSequenceEmu.offsets[0],
		&cmdlist.tokenSequenceEmu.sizes[0],
		&cmdlist.tokenSequenceEmu.states[0],
		&cmdlist.tokenSequenceEmu.fbos[0],
		GLuint(cmdlist.tokenSequenceEmu.offsets.size()),
		cmdlist.statesystem);

	if (bindlessVboUbo) {
		glDisableClientState(GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV);
		glDisableClientState(GL_ELEMENT_ARRAY_UNIFIED_NV);
		glDisableClientState(GL_UNIFORM_BUFFER_UNIFIED_NV);

#if _DEBUG
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif
	}
}


auto settingsFunc = [](App::Settings *settings)
{
	//settings->setHighDensityDisplayEnabled();
	settings->prepareWindow(Window::Format().size(vec2(800, 600)).title("NV_Compute_Particles"));
#ifdef DISABLE_FRAMERATE
	settings->disableFrameRate();
#else
	settings->setFrameRate(60.0f);
#endif //DISABLE_FRAMERATE
	//#if defined( CINDER_MSW )//#endif
	//settings->setConsoleWindowEnabled();
};


CINDER_APP(NVidiaCommandListApp, RendererGl(ci::app::RendererGl::Options().version(4, 3)), settingsFunc)
