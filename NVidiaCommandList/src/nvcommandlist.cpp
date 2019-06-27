//----------------------------------------------------------------------------------
// File:        gl4-maxwell\NvCommandList/nvcommandlist.cpp
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

#include "nvcommandlist.h"
/*
PFNGLCREATESTATESNVPROC glad_glCreateStatesNV;
PFNGLDELETESTATESNVPROC glad_glDeleteStatesNV;
PFNGLISSTATENVPROC glad_glIsStateNV;
PFNGLSTATECAPTURENVPROC glad_glStateCaptureNV;
PFNGLDRAWCOMMANDSNVPROC glad_glDrawCommandsNV;
PFNGLDRAWCOMMANDSADDRESSNVPROC glad_glDrawCommandsAddressNV;
PFNGLDRAWCOMMANDSSTATESNVPROC glad_glDrawCommandsStatesNV;
PFNGLDRAWCOMMANDSSTATESADDRESSNVPROC glad_glDrawCommandsStatesAddressNV;
PFNGLCREATECOMMANDLISTSNVPROC glad_glCreateCommandListsNV;
PFNGLDELETECOMMANDLISTSNVPROC glad_glDeleteCommandListsNV;
PFNGLISCOMMANDLISTNVPROC glad_glIsCommandListNV;
PFNGLLISTDRAWCOMMANDSSTATESCLIENTNVPROC glad_glListDrawCommandsStatesClientNV;
PFNGLCOMMANDLISTSEGMENTSNVPROC glad_glCommandListSegmentsNV;
PFNGLCOMPILECOMMANDLISTNVPROC glad_glCompileCommandListNV;
PFNGLCALLCOMMANDLISTNVPROC glad_glCallCommandListNV;
PFNGLGETCOMMANDHEADERNVPROC glad_glGetCommandHeaderNV;
PFNGLGETSTAGEINDEXNVPROC glad_glGetStageIndexNV;
*/

static int initedNVcommandList = 0;

int init_NV_command_list(NVCPROC (*fnGetProc)(const char* name))
{
  if (initedNVcommandList) return glad_glCreateStatesNV != ((void*)0);

  glad_glCreateStatesNV = (PFNGLCREATESTATESNVPROC)fnGetProc("glCreateStatesNV");
  glad_glDeleteStatesNV = (PFNGLDELETESTATESNVPROC)fnGetProc("glDeleteStatesNV");
  glad_glIsStateNV = (PFNGLISSTATENVPROC)fnGetProc("glIsStateNV");
  glad_glStateCaptureNV = (PFNGLSTATECAPTURENVPROC)fnGetProc("glStateCaptureNV");
  glad_glDrawCommandsNV = (PFNGLDRAWCOMMANDSNVPROC)fnGetProc("glDrawCommandsNV");
  glad_glDrawCommandsAddressNV = (PFNGLDRAWCOMMANDSADDRESSNVPROC)fnGetProc("glDrawCommandsAddressNV");
  glad_glDrawCommandsStatesNV = (PFNGLDRAWCOMMANDSSTATESNVPROC)fnGetProc("glDrawCommandsStatesNV");
  glad_glDrawCommandsStatesAddressNV = (PFNGLDRAWCOMMANDSSTATESADDRESSNVPROC)fnGetProc("glDrawCommandsStatesAddressNV");
  glad_glCreateCommandListsNV = (PFNGLCREATECOMMANDLISTSNVPROC)fnGetProc("glCreateCommandListsNV");
  glad_glDeleteCommandListsNV = (PFNGLDELETECOMMANDLISTSNVPROC)fnGetProc("glDeleteCommandListsNV");
  glad_glIsCommandListNV = (PFNGLISCOMMANDLISTNVPROC)fnGetProc("glIsCommandListNV");
  glad_glListDrawCommandsStatesClientNV = (PFNGLLISTDRAWCOMMANDSSTATESCLIENTNVPROC)fnGetProc("glListDrawCommandsStatesClientNV");
  glad_glCommandListSegmentsNV = (PFNGLCOMMANDLISTSEGMENTSNVPROC)fnGetProc("glCommandListSegmentsNV");
  glad_glCompileCommandListNV = (PFNGLCOMPILECOMMANDLISTNVPROC)fnGetProc("glCompileCommandListNV");
  glad_glCallCommandListNV = (PFNGLCALLCOMMANDLISTNVPROC)fnGetProc("glCallCommandListNV");
  glad_glGetCommandHeaderNV = (PFNGLGETCOMMANDHEADERNVPROC)fnGetProc("glGetCommandHeaderNV");
  glad_glGetStageIndexNV = (PFNGLGETSTAGEINDEXNVPROC)fnGetProc("glGetStageIndexNV");
  
  initedNVcommandList = 1;
  
  return glad_glCreateStatesNV != ((void*)0);
}

