#pragma once

#include <map>
#include "Engine/Math/Vector2Int.hpp"

class Texture
{
public:
	enum class TextureFormat
	{
		RGBA8, //RGBA, 8 bits per channel
		D24S8, //Depth 24, Stencil 8
		NUM_FORMATS
	};
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	Texture(uint32_t width, uint32_t height, TextureFormat format);
	~Texture();
	static Texture* CreateOrGetTexture(const std::string& imageFilePath);
	static Texture* CreateTextureFromData(const std::string& textureName, unsigned char* textureData, int numComponents, const Vector2Int& texelSize);

	//GETTERS//////////////////////////////////////////////////////////////////////////
	static Texture* GetTextureByName(const std::string& imageFilePath);
	unsigned char* GetImageData();

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	unsigned int m_openglTextureID;
	Vector2Int m_texelSize;

private:
	Texture(const std::string& imageFilePath);
	Texture(unsigned char* textureData, int numColorComponents, const Vector2Int& texelSize);

	static std::map<std::string, Texture*> s_textureRegistry;
	unsigned char* m_imageData;
};
