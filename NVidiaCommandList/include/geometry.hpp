//----------------------------------------------------------------------------------
// File:        gl4-maxwell\NvCommandList/geometry.hpp
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

#ifndef NV_GEOMETRY_INCLUDED
#define NV_GEOMETRY_INCLUDED

#include <vector>
//#include "NV/NvMatrix.h"
//#include "NV/NvMath.h"
#include "cinder/app/App.h"
//#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#define nv_pi  float(3.14159265358979323846264338327950288419716939937510582)

namespace geometry {
    struct Vertex
    {
      Vertex
        (
        ci::vec3 const & position,
        ci::vec3 const & normal,
        ci::vec2 const & texcoord
        ) :
          position( ci::vec4(position,1.0f)),
          normal  ( ci::vec4(normal,0.0f)),
          texcoord( ci::vec4(texcoord,0.0f,0.0f))
      {}

      ci::vec4 position;
      ci::vec4 normal;
      ci::vec4 texcoord;
    };

    template <class TVertex>
    class Mesh {
    public:
      std::vector<TVertex>        m_vertices;
      std::vector<ci::ivec3 >     m_indicesTriangles;
      std::vector<ci::ivec2 >     m_indicesOutline;

      void append(Mesh<TVertex>& geo)
      {
        m_vertices.reserve(geo.m_vertices.size() + m_vertices.size());
        m_indicesTriangles.reserve(geo.m_indicesTriangles.size() + m_indicesTriangles.size());
        m_indicesOutline.reserve(geo.m_indicesOutline.size() + m_indicesOutline.size());

        unsigned int offset = (unsigned int)(m_vertices.size());

        for (size_t i = 0; i < geo.m_vertices.size(); i++){
          m_vertices.push_back(geo.m_vertices[i]);
        }

        for (size_t i = 0; i < geo.m_indicesTriangles.size(); i++){
          m_indicesTriangles.push_back(geo.m_indicesTriangles[i] + ci::ivec3(offset));
        }

        for (size_t i = 0; i < geo.m_indicesOutline.size(); i++){
          m_indicesOutline.push_back(geo.m_indicesOutline[i] +  ci::ivec2(offset));
        }
      }

      void flipWinding()
      {
        for (size_t i = 0; i < m_indicesTriangles.size(); i++){
          std::swap(m_indicesTriangles[i].x,m_indicesTriangles[i].z);
        }
      }

      size_t getTriangleIndicesSize() const{
        return m_indicesTriangles.size() * sizeof(ci::ivec3);
      }

      unsigned int getTriangleIndicesCount() const{
        return (unsigned int)m_indicesTriangles.size() * 3;
      }

      size_t getOutlineIndicesSize() const{
        return m_indicesOutline.size() * sizeof(ci::ivec2);
      }

      unsigned int getOutlineIndicesCount() const{
        return (unsigned int)m_indicesOutline.size() * 2;
      }

      size_t getVerticesSize() const{
        return m_vertices.size() * sizeof(TVertex);
      }

      unsigned int getVerticesCount() const{
        return (unsigned int)m_vertices.size();
      }
    };

    template <class TVertex>
    class Plane : public Mesh<TVertex> {
    public:
      static void add(Mesh<TVertex>& geo, const glm::mat4& mat, int w, int h)
      {
        int xdim = w;
        int ydim = h;

        float xmove = 1.0f/(float)xdim;
        float ymove = 1.0f/(float)ydim;

        int width = (xdim + 1);

        unsigned int vertOffset = (unsigned int)geo.m_vertices.size();

        int x,y;
        for (y = 0; y < ydim + 1; y++){
          for (x = 0; x < xdim + 1; x++){
            float xpos = ((float)x * xmove);
            float ypos = ((float)y * ymove);
            ci::vec3 pos;
            ci::vec2 uv;
            ci::vec3 normal;

            pos[0] = (xpos - 0.5f) * 2.0f;
            pos[1] = (ypos - 0.5f) * 2.0f;
            pos[2] = 0;

            uv[0] = xpos;
            uv[1] = ypos;

            normal[0] = 0.0f;
            normal[1] = 0.0f;
            normal[2] = 1.0f;

            Vertex vert = Vertex(pos,normal,uv);
            vert.position = mat * vert.position;
            vert.normal   = mat * vert.normal;
            geo.m_vertices.push_back(TVertex(vert));
          }
        }

        for (y = 0; y < ydim; y++){
          for (x = 0; x < xdim; x++){
            // upper tris
            geo.m_indicesTriangles.push_back(
              ci::ivec3(
              (x)      + (y + 1) * width + vertOffset,
              (x)      + (y)     * width + vertOffset,
              (x + 1)  + (y + 1) * width + vertOffset
              )
              );
            // lower tris
            geo.m_indicesTriangles.push_back(
              ci::ivec3(
              (x + 1)  + (y + 1) * width + vertOffset,
              (x)      + (y)     * width + vertOffset,
              (x + 1)  + (y)     * width + vertOffset
              )
              );
          }
        }

        for (y = 0; y < ydim; y++){
          geo.m_indicesOutline.push_back(
            ci::ivec2(
            (y)     * width + vertOffset,
            (y + 1) * width + vertOffset
            )
            );
        }
        for (y = 0; y < ydim; y++){
          geo.m_indicesOutline.push_back(
            ci::ivec2(
            (y)     * width + xdim + vertOffset,
            (y + 1) * width + xdim + vertOffset
            )
            );
        }
        for (x = 0; x < xdim; x++){
          geo.m_indicesOutline.push_back(
            ci::ivec2(
            (x)     + vertOffset,
            (x + 1) + vertOffset
            )
            );
        }
        for (x = 0; x < xdim; x++){
          geo.m_indicesOutline.push_back(
            ci::ivec2(
            (x)     + ydim * width + vertOffset,
            (x + 1) + ydim * width + vertOffset
            )
            );
        }
      }

      Plane (int segments = 1 ){
		glm::mat4 mat = mat4(1.);
//		mat.make_identity();

        add(*this,mat,segments,segments);
      }
    };

    template <class TVertex>
    class Box : public Mesh<TVertex> {
    public:
      static void add(Mesh<TVertex>& geo, const glm::mat4& mat, int w, int h, int d)
      {
        int configs[6][2] = {
          {w,h},
          {w,h},

          {d,h},
          {d,h},

          {w,d},
          {w,d},
        };

        for (int side = 0; side < 6; side++){
          glm::mat4 matrixRot=mat4(1.);
//		  matrixRot.make_identity();

          switch (side)
          {
          case 0:
            break;
          case 1:
			  //nv::rotationY(matrixRot, nv_pi);
			  //nv::rotationY(matrixRot, nv_pi);
            break;
          case 2:
            //nv::rotationY(matrixRot, nv_pi * 0.5f);
            break;
          case 3:
            //nv::rotationY(matrixRot, nv_pi * 1.5f);
            break;
          case 4:
            //nv::rotationX(matrixRot, nv_pi * 0.5f);
            break;
          case 5:
            //nv::rotationX(matrixRot, nv_pi * 1.5f);
            break;
          }

          glm::mat4 matrixMove;
		  //nv::translation(matrixMove, 0.0f, 0.0f, 1.0f);

          Plane<TVertex>::add(geo, mat * matrixRot * matrixMove,configs[side][0],configs[side][1]);
        }
      }

      Box (int segments = 1 ){
		glm::mat4 mat=mat4(1.);
		//mat.make_identity();

        add(*this, mat /*glm::mat4(1)*/,segments,segments,segments);
      }
    };

    template <class TVertex>
    class Sphere : public Mesh<TVertex> {
    public:
      static void add(Mesh<TVertex>& geo, const glm::mat4& mat, int w, int h)
      {
        int xydim = w;
        int zdim  = h;

        unsigned int vertOffset = (unsigned int)geo.m_vertices.size();

        float xyshift = 1.0f / (float)xydim;
        float zshift  = 1.0f / (float)zdim;
        int width = xydim + 1;

        int xy,z;
        for (z = 0; z < zdim + 1; z++){
          for (xy = 0; xy < xydim + 1; xy++){
            ci::vec3 pos;
            ci::vec3 normal;
            ci::vec2 uv;
            float curxy = xyshift * (float)xy;
            float curz  = zshift  * (float)z;
            float anglexy = curxy * nv_pi * 2.0f;
            float anglez  = (1.0f-curz) * nv_pi;
            pos[0] = cosf(anglexy) * sinf(anglez);
            pos[1] = sinf(anglexy) * sinf(anglez);
            pos[2] = cosf(anglez);
            normal = pos;
            uv[0]  = curxy;
            uv[1]  = curz;

            Vertex vert = Vertex(pos,normal,uv);
            vert.position = mat * vert.position;
            vert.normal   = mat * vert.normal;

            geo.m_vertices.push_back(TVertex(vert));
          }
        }

        int vertex = 0;
        for (z = 0; z < zdim; z++){
          for (xy = 0; xy < xydim; xy++, vertex++){
            ci::ivec3 indices;
            if (z != zdim-1){
              indices[2] = vertex + vertOffset;
              indices[1] = vertex + width + vertOffset;
              indices[0] = vertex + width + 1 + vertOffset;
              geo.m_indicesTriangles.push_back(indices);
            }

            if (z != 0){
              indices[2] = vertex + width + 1 + vertOffset;
              indices[1] = vertex + 1 + vertOffset;
              indices[0] = vertex + vertOffset;
              geo.m_indicesTriangles.push_back(indices);
            }

          }
          vertex++;
        }

        int middlez = zdim / 2;

        for (xy = 0; xy < xydim; xy++){
          ci::ivec2 indices;
          indices[0] = middlez * width + xy + vertOffset;
          indices[1] = middlez * width + xy + 1 + vertOffset;
          geo.m_indicesOutline.push_back(indices);
        }

        for (int i = 0; i < 4; i++){
          int x = (xydim * i) / 4;
          for (z = 0; z < zdim; z++){
            ci::ivec2 indices;
            indices[0] = x + width * (z) + vertOffset;
            indices[1] = x + width * (z + 1) + vertOffset;
            geo.m_indicesOutline.push_back(indices);
          }
        }
      }

      Sphere (int w=16, int h=8 ){
		glm::mat4 mat=mat4(1.);
		//mat.make_identity();

        add(*this, mat,w,h);
      }
    };
 }



#endif

