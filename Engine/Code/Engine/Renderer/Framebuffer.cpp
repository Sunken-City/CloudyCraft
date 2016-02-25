#include "Engine/Renderer/Framebuffer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include "Engine/Renderer/OpenGLExtensions.hpp"
#include "Engine/Renderer/Texture.hpp"

//-----------------------------------------------------------------------------------
Framebuffer* Framebuffer::FramebufferCreate(size_t colorCount, Texture** inColorTargets, Texture* depthStencilTarget)
{
	ASSERT_OR_DIE(colorCount > 0, "Color count was negative");
	Texture* color0 = inColorTargets[0];
	uint32_t width = color0->m_texelSize.x;
	uint32_t height = color0->m_texelSize.y;

	for (uint32_t i = 0; i < colorCount; ++i)
	{
		Texture* color = inColorTargets[i];
		ASSERT_OR_DIE((color->m_texelSize.x == width) && (color->m_texelSize.y == height), "Color target didn't match the height and width of the first target");
	}

	if (nullptr != depthStencilTarget)
	{
		ASSERT_OR_DIE(((uint32_t)depthStencilTarget->m_texelSize.x == width) && ((uint32_t)depthStencilTarget->m_texelSize.y == height), "Depth Stencil Target didn't match the height and width of the first target");
	}

	GLuint fboHandle;
	glGenFramebuffers(1, &fboHandle);
	ASSERT_OR_DIE(fboHandle != NULL, "Failed to grab fbo handle");

	Framebuffer* fbo = new Framebuffer();
	fbo->m_fboHandle = fboHandle;
	fbo->m_colorCount = colorCount;
	fbo->m_pixelWidth = width;
	fbo->m_pixelHeight = height;


	for (uint32_t i = 0; i < colorCount; ++i)
	{
		fbo->m_colorTargets.push_back(inColorTargets[i]);
	}
	fbo->m_depthStencilTarget = depthStencilTarget;

	//OpenGL initialization stuff
	//If you bound a framebuffer to your Renderer, be careful you didn't unbind just now...
	glBindFramebuffer(GL_FRAMEBUFFER, fbo->m_fboHandle);

	//Bind our color targets to our FBO
	for (uint32_t i = 0; i < colorCount; ++i)
	{
		Texture* tex = inColorTargets[i];
		glFramebufferTexture(GL_FRAMEBUFFER, //What we're attaching
			GL_COLOR_ATTACHMENT0 + i, //Where we're attaching
			tex->m_openglTextureID, //OpenGL id
			0); //Level - probably mipmap level
	}
	
	//Bind depth stencil if you have it.
	if (nullptr != depthStencilTarget)
	{
		glFramebufferTexture(GL_FRAMEBUFFER,
			GL_DEPTH_STENCIL_ATTACHMENT,
			depthStencilTarget->m_openglTextureID,
			0);
	}

	//Make sure everything was bound correctly, no errors!
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		FramebufferDelete(fbo);
		ERROR_RECOVERABLE("Error occured while binding framebuffer");
		return nullptr;
	}

	//Revert to old state
	glBindFramebuffer(GL_FRAMEBUFFER, NULL);
	return fbo;
}

Framebuffer* Framebuffer::FramebufferCreate(uint32_t width, uint32_t height, std::vector<Texture::TextureFormat> colorFormats, Texture::TextureFormat depthStencilFormat)
{
	//1. do thing
	return nullptr;
}

void Framebuffer::FramebufferDelete(Framebuffer *fbo)
{
	if (Renderer::instance->m_fbo == fbo)
	{
		Renderer::instance->BindFramebuffer(nullptr);
	}
	glDeleteFramebuffers(1, &fbo->m_fboHandle);
	delete fbo;
}
