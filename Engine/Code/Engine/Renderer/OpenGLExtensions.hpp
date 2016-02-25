#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLu.h>
#pragma comment(lib, "opengl32") //Link in the OpenGL32.lib static library
#pragma comment(lib, "GLu32") //Link in the GLu32.lib static library
#include "ThirdParty/OpenGL/glext.h"
#include "ThirdParty/OpenGL/wglext.h"

extern PFNGLGENBUFFERSPROC	glGenBuffers;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;

extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;

extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLDETACHSHADERPROC glDetachShader;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;

extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLUSEPROGRAMPROC glUseProgram;

extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform;
extern PFNGLUNIFORM1FVPROC	glUniform1fv;
extern PFNGLUNIFORM2FVPROC	glUniform2fv;
extern PFNGLUNIFORM3FVPROC	glUniform3fv;
extern PFNGLUNIFORM4FVPROC	glUniform4fv;
extern PFNGLUNIFORM1IVPROC	glUniform1iv;
extern PFNGLUNIFORM2IVPROC	glUniform2iv;
extern PFNGLUNIFORM3IVPROC	glUniform3iv;
extern PFNGLUNIFORM4IVPROC	glUniform4iv;

extern PFNGLBINDSAMPLERPROC glBindSampler;
extern PFNGLGENSAMPLERSPROC glGenSamplers;
extern PFNGLSAMPLERPARAMETERIPROC glSamplerParameteri;
extern PFNGLDELETESAMPLERSPROC	glDeleteSamplers;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
extern PFNGLACTIVETEXTUREPROC glActiveTexture;

extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
extern PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
extern PFNGLDRAWBUFFERSPROC glDrawBuffers;
extern PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;

void hookUpOpenGLPointers();