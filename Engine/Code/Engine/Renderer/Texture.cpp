//---------------------------------------------------------------------------
// Based on code written by Squirrel Eiserloh
//
#include "Engine/Renderer/Texture.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include <gl/GLU.h>
#include "Engine/Renderer/OpenGLExtensions.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"

#define STBI_HEADER_FILE_ONLY
#include "ThirdParty/stb_image.c"

#define STATIC // Do-nothing indicator that method/member is static in class definition

//---------------------------------------------------------------------------
STATIC std::map<size_t, Texture*, std::less<size_t>, UntrackedAllocator<std::pair<size_t, Texture*>>> Texture::s_textureRegistry;

//-----------------------------------------------------------------------------------
STATIC void Texture::CleanUpTextureRegistry()
{
    for (auto texturePair : s_textureRegistry)
    {
        delete texturePair.second;
    }
    s_textureRegistry.clear();
}

//---------------------------------------------------------------------------
Texture::Texture(const std::string& imageFilePath)
    : m_openglTextureID(0)
    , m_texelSize(0, 0)
    , m_imageData(nullptr)
    , m_initializationMethod(TextureInitializationMethod::FROM_DISK)
    , m_textureFormat(TextureFormat::NUM_FORMATS)
{
    int numComponents = 0; // Filled in for us to indicate how many color/alpha components the image had (e.g. 3=RGB, 4=RGBA)
    int numComponentsRequested = 0; // don't care; we support 3 (RGB) or 4 (RGBA)
    m_imageData = stbi_load( imageFilePath.c_str(), &m_texelSize.x, &m_texelSize.y, &numComponents, numComponentsRequested );

    // Enable texturing
    glEnable( GL_TEXTURE_2D );

    // Tell OpenGL that our pixel data is single-byte aligned
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    // Ask OpenGL for an unused texName (ID number) to use for this texture
    glGenTextures( 1, (GLuint*) &m_openglTextureID );

    // Tell OpenGL to bind (set) this as the currently active texture
    glBindTexture( GL_TEXTURE_2D, m_openglTextureID );

    // Set texture clamp vs. wrap (repeat)
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP ); // one of: GL_CLAMP or GL_REPEAT
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP ); // one of: GL_CLAMP or GL_REPEAT

    // Set magnification (texel > pixel) and minification (texel < pixel) filters
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );  // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR

    GLenum bufferFormat = GL_RGBA; // the format our source pixel data is currently in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
    if( numComponents == 3 )
        bufferFormat = GL_RGB;

    // #FIXME: What happens if numComponents is neither 3 nor 4?

    GLenum internalFormat = bufferFormat; // the format we want the texture to me on the card; allows us to translate into a different texture format as we upload to OpenGL

    glTexImage2D(			// Upload this pixel data to our new OpenGL texture
        GL_TEXTURE_2D,		// Creating this as a 2d texture
        0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
        internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
        m_texelSize.x,		// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,10], and B is the border thickness [0,1]
        m_texelSize.y,		// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,10], and B is the border thickness [0,1]
        0,					// Border size, in texels (must be 0 or 1)
        bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
        GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color/alpha channel)
        m_imageData );		// Location of the actual pixel data bytes/buffer

    glDisable(GL_TEXTURE_2D);
}

//-----------------------------------------------------------------------------------
Texture::Texture(unsigned char* textureData, int numColorComponents, const Vector2Int& texelSize)
    : m_openglTextureID(0)
    , m_texelSize(texelSize.x, texelSize.y)
    , m_imageData(textureData)
    , m_initializationMethod(TextureInitializationMethod::FROM_MEMORY)
    , m_textureFormat(TextureFormat::NUM_FORMATS)
{
    int numComponents = numColorComponents; // Filled in for us to indicate how many color/alpha components the image had (e.g. 3=RGB, 4=RGBA)

    // Enable texturing
    glEnable(GL_TEXTURE_2D);

    // Tell OpenGL that our pixel data is single-byte aligned
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Ask OpenGL for an unused texName (ID number) to use for this texture
    glGenTextures(1, (GLuint*)&m_openglTextureID);

    // Tell OpenGL to bind (set) this as the currently active texture
    glBindTexture(GL_TEXTURE_2D, m_openglTextureID);

    // Set texture clamp vs. wrap (repeat)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP); // one of: GL_CLAMP or GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); // one of: GL_CLAMP or GL_REPEAT

    // Set magnification (texel > pixel) and minification (texel < pixel) filters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // one of: GL_NEAREST, GL_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR

    GLenum bufferFormat = GL_RGBA; // the format our source pixel data is currently in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
    if (numComponents == 3)
        bufferFormat = GL_RGB;

    // #FIXME: What happens if numComponents is neither 3 nor 4?

    GLenum internalFormat = bufferFormat; // the format we want the texture to me on the card; allows us to translate into a different texture format as we upload to OpenGL

    glTexImage2D(			// Upload this pixel data to our new OpenGL texture
        GL_TEXTURE_2D,		// Creating this as a 2d texture
        0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
        internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
        m_texelSize.x,		// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,10], and B is the border thickness [0,1]
        m_texelSize.y,		// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,10], and B is the border thickness [0,1]
        0,					// Border size, in texels (must be 0 or 1)
        bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
        GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color/alpha channel)
        m_imageData);		// Location of the actual pixel data bytes/buffer

    glDisable(GL_TEXTURE_2D);
}

Texture::Texture(uint32_t width, uint32_t height, TextureFormat format)
    : m_initializationMethod(TextureInitializationMethod::FROM_MEMORY)
    , m_texelSize(width, height)
    , m_textureFormat(format)
{
    glGenTextures(1, &m_openglTextureID);
    GLenum bufferChannels = GL_RGBA;
    GLenum bufferFormat = GL_UNSIGNED_INT_8_8_8_8;
    GLenum internalFormat = GL_RGBA8;

    if (format == TextureFormat::RGBA8)
    {
        //Nothing changes
    }
    else if (format == TextureFormat::D24S8)
    {
        bufferChannels = GL_DEPTH_STENCIL;
        bufferFormat = GL_UNSIGNED_INT_24_8;
        internalFormat = GL_DEPTH24_STENCIL8;
    }
    else if (format == TextureFormat::R32UI)
    {
        bufferChannels = GL_RED_INTEGER;
        bufferFormat = GL_UNSIGNED_INT;
        internalFormat = GL_R32UI;
    }
    else
    {
        ERROR_AND_DIE("Unsupported texture enum");
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_openglTextureID);
    glTexImage2D(GL_TEXTURE_2D,
        0, //level - mipmaplevel, set to 0,
        internalFormat, //How texture is stored in memory
        width, height,
        0, //border, again set to 0, we want not 0
        bufferChannels, //channels used by image pass in
        bufferFormat, //format of data of image passed in
        NULL);	//no actual data passed in, defaults black/white

    GL_CHECK_ERROR();
}

//-----------------------------------------------------------------------------------
unsigned char* Texture::GetImageData()
{
    return m_imageData;
}

//-----------------------------------------------------------------------------------
Texture::~Texture()
{
    glDeleteTextures(1, &m_openglTextureID);
    if (m_imageData)
    {
        switch (m_initializationMethod)
        {
        //We have an additional step for STBI loaded images
        case TextureInitializationMethod::FROM_DISK:
            stbi_image_free(m_imageData);
            break;
        case TextureInitializationMethod::FROM_MEMORY:
            break;
        case TextureInitializationMethod::NUM_INITIALIZATION_METHODS:
            break;
        default:
            break;
        }
    }
}

//---------------------------------------------------------------------------
// Returns a pointer to the already-loaded texture of a given image file,
//	or nullptr if no such texture/image has been loaded.
//
STATIC Texture* Texture::GetTextureByName(const std::string& imageFilePath)
{
    size_t filePathHash = std::hash<std::string>{}(imageFilePath);
    auto iterator = Texture::s_textureRegistry.find(filePathHash);
    if (iterator == Texture::s_textureRegistry.end())
    {
        return nullptr;
    }
    else
    {
        return iterator->second;
    }
}


//---------------------------------------------------------------------------
// Finds the named Texture among the registry of those already loaded; if
//	found, returns that Texture*.  If not, attempts to load that texture,
//	and returns a Texture* just created (or nullptr if unable to load file).
//
STATIC Texture* Texture::CreateOrGetTexture(const std::string& imageFilePath)
{
    Texture* texture = GetTextureByName(imageFilePath);
    if (texture != nullptr)
    {
        return texture;
    }
    else
    {
        texture = new Texture(imageFilePath);
        size_t filePathHash = std::hash<std::string>{}(imageFilePath);
        Texture::s_textureRegistry[filePathHash] = texture;
        return texture;
    }
}

//-----------------------------------------------------------------------------------
Texture* Texture::CreateTextureFromData(const std::string& textureName, unsigned char* textureData, int numComponents, const Vector2Int& texelSize)
{
    Texture* texture = new Texture(textureData, numComponents, texelSize);
    size_t stringHash = std::hash<std::string>{}(textureName);
    Texture::s_textureRegistry[stringHash] = texture;
    return texture;
}

