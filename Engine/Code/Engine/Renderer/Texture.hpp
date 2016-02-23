#pragma once

#include <map>
#include "Engine/Math/Vector2Int.hpp"

class Texture
{
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	~Texture();
	static Texture* CreateOrGetTexture(const std::string& imageFilePath);
	static Texture* CreateTextureFromData(const std::string& textureName, unsigned char* textureData, int numComponents, const Vector2Int& texelSize);

	//GETTERS//////////////////////////////////////////////////////////////////////////
	static Texture* GetTextureByName(const std::string& imageFilePath);
	unsigned char* GetImageData();

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	int m_openglTextureID;
	Vector2Int m_texelSize;
private:
	Texture(const std::string& imageFilePath);
	Texture(unsigned char* textureData, int numColorComponents, const Vector2Int& texelSize);

	static std::map<std::string, Texture*> s_textureRegistry;
	unsigned char* m_imageData;
};
