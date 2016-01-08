#pragma once

#include <map>
#include "Engine/Math/Vector2Int.hpp"

class Texture
{
public:
	static Texture* GetTextureByName(const std::string& imageFilePath);
	static Texture* CreateOrGetTexture(const std::string& imageFilePath);
	static Texture* CreateTextureFromData(const std::string& textureName, unsigned char* textureData, int numComponents, const Vector2Int& texelSize);
	int m_openglTextureID;
	Vector2Int m_texelSize;
	Texture(const std::string& imageFilePath, unsigned char* textureData, int numColorComponents, const Vector2Int& texelSize);
	unsigned char* GetImageData();
	~Texture();

private:
	Texture(const std::string& imageFilePath);
	static std::map<std::string, Texture*> s_textureRegistry;
	unsigned char* m_imageData;
};
