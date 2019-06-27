//----------------------------------------------------------------------------------
// File:        gl4-maxwell\NvCommandList/common.h
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
#include "geometry.hpp"
#include "nvtoken.hpp"
#include "common.h"

using namespace std;
using namespace nvtoken;
using namespace basiccmdlist;

struct ShaderSourceItem {
	const char* src; ///< Null-terminated string containing the shader source code
	GLint type; ///< The GL_*_SHADER enum representing the shader type
};

struct GLSLProgram {

private:
	GLuint program;

public:
	GLuint getProgram() { return this->program; }
	void setProgram(GLuint id) { this->program = id; }
};


enum DrawMode {
	DRAW_STANDARD,
	DRAW_TOKEN_EMULATED,
	DRAW_TOKEN_BUFFER,
	DRAW_TOKEN_LIST,
};

struct comp_programs {
	size_t
		draw_scene,
		draw_scene_geo;
} programs;

struct comp_textures {
	GLuint
		scene_color,
		scene_depthstencil,
		color;
}textures;

struct comp_texturesADDR {
	GLuint64
		scene_color,
		scene_depthstencil,
		color;
}texturesADDR;

struct comp_fbos {
	GLuint
		scene;
}fbos;

struct comp_buffers {
	GLuint
		box_vbo,
		box_ibo,
		sphere_vbo,
		sphere_ibo,

		scene_ubo,
		objects_ubo;
} buffers;

struct comp_buffersADDR {
	GLuint64
		box_vbo,
		box_ibo,
		sphere_vbo,
		sphere_ibo,

		scene_ubo,
		objects_ubo;
} buffersADDR;

struct Vertex {

	Vertex(const geometry::Vertex& vertex) {
		position = vertex.position;
		normal[0] = short(vertex.normal.x * float(32767));
		normal[1] = short(vertex.normal.y * float(32767));
		normal[2] = short(vertex.normal.z * float(32767));
		uv = ci::vec2(vertex.texcoord);
	}

	ci::vec4     position;
	short         normal[4];
	ci::vec2     uv;
};

struct ObjectInfo {
	GLuint    vbo;
	GLuint    ibo;
	GLuint64  vboADDR;
	GLuint64  iboADDR;
	GLuint    numIndices;
	size_t   program;
};

// COMMANDLIST
struct StateIncarnation {
	unsigned int  programIncarnation;
	unsigned int  fboIncarnation;

	bool operator ==(const StateIncarnation& other) const
	{
		return memcmp(this, &other, sizeof(StateIncarnation)) == 0;
	}

	bool operator !=(const StateIncarnation& other) const
	{
		return memcmp(this, &other, sizeof(StateIncarnation)) != 0;
	}

	StateIncarnation()
		: programIncarnation(0)
		, fboIncarnation(0)
	{

	}
};
struct CmdList {
	// for emulation
	StateSystem       statesystem;

	// we introduce variables that track when we changed global state
	StateIncarnation  state;
	StateIncarnation  captured;

	// two state objects
	GLuint                stateobj_draw;
	GLuint                stateobj_draw_geo;
	StateSystem::StateID  stateid_draw;
	StateSystem::StateID  stateid_draw_geo;


	// there is multiple ways to draw the scene
	// either via buffer, cmdlist object, or emulation
	GLuint          tokenBuffer;
	GLuint          tokenCmdList;
	std::string     tokenData;
	NVTokenSequence tokenSequence;
	NVTokenSequence tokenSequenceList;
	NVTokenSequence tokenSequenceEmu;
} cmdlist;

struct Tweak {
	DrawMode    mode;
	ci::vec3   lightDir;

	Tweak()
		: mode(DRAW_STANDARD)
	{

	}
};

