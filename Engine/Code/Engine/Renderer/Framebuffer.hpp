#pragma once
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include <vector>

class Framebuffer
{
	typedef unsigned int GLuint;
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	static Framebuffer* FramebufferCreate(size_t colorCount, Texture** colorTargets, Texture* depthStencilTarget);
	static Framebuffer* FramebufferCreate(uint32_t width, uint32_t height, std::vector<Texture::TextureFormat> colorFormats, Texture::TextureFormat depthStencilFormat);
	static void FramebufferDelete(Framebuffer *fbo);
	
	//MEMBER VARIABLES/////////////////////////////////////////////////////////////////////////
	GLuint m_fboHandle;
	std::vector<Texture*> m_colorTargets;
	Texture* m_depthStencilTarget;
	size_t m_colorCount;
	uint32_t m_pixelWidth;
	uint32_t m_pixelHeight;
};