#pragma once
#include <string>
#include "Engine/Renderer/ShaderProgram.hpp"

class Matrix4x4;
class Texture;

//STRUCTS//////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct RenderState
{
    //ENUMS//////////////////////////////////////////////////////////////////////////
    enum class DepthTestingMode
    {
        ON,
        OFF,
        DISABLE_WRITE,
        //XRAY,
        NUM_MODES
    };

    enum class FaceCullingMode
    {
        CULL_BACK_FACES,
        RENDER_BACK_FACES,
        NUM_MODES
    };

    enum class BlendMode
    {
        ADDITIVE_BLEND,
        ALPHA_BLEND,
        INVERTED_BLEND,
        NUM_MODES
    };
    
    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    RenderState(DepthTestingMode depthTesting = DepthTestingMode::ON, FaceCullingMode faceCulling = FaceCullingMode::CULL_BACK_FACES, BlendMode blendMode = BlendMode::ALPHA_BLEND);
    ~RenderState();

    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    void SetState() const;
    void ClearState() const;

    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    DepthTestingMode depthTestingMode;
    FaceCullingMode faceCullingMode;
    BlendMode blendMode;
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Material
{
public:
    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    Material() {};
    Material(ShaderProgram* program, const RenderState& renderState);
    ~Material();

    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    void SetMatrices(const Matrix4x4& model, const Matrix4x4& view, const Matrix4x4& projection);
    void BindAvailableTextures() const;
    void UnbindAvailableTextures() const;
    void SetDiffuseTexture(const std::string& diffusePath);
    void SetDiffuseTexture(Texture* texture);
    void SetNormalTexture(const std::string& normalPath);
    void SetNormalTexture(Texture* texture);
    void SetEmissiveTexture(const std::string& emissivePath);
    void SetEmissiveTexture(Texture* texture);
    void SetNoiseTexture(const std::string& noisePath);
    void SetNoiseTexture(Texture* texture);
    void SetTexture(const char* texName, unsigned int textureObjectID);
    void SetUpRenderState() const;
    void CleanUpRenderState() const;
    inline void SetVec4Uniform(const char* name, const Vector4& value) { m_shaderProgram->SetVec4Uniform(name, value); };
    inline void SetVec3Uniform(const char* name, const Vector3& value) { m_shaderProgram->SetVec3Uniform(name, value); };
    inline void SetFloatUniform(const char* name, float value) { m_shaderProgram->SetFloatUniform(name, value); };
    inline void SetIntUniform(const char* name, int value) { m_shaderProgram->SetIntUniform(name, value); };
    inline void SetMatrix4x4Uniform(const char* name, Matrix4x4& value, unsigned int arrayIndex) { m_shaderProgram->SetMatrix4x4Uniform(name, value, arrayIndex); };
    inline void SetVec4Uniform(const char* name, const Vector4& value, unsigned int arrayIndex) { m_shaderProgram->SetVec4Uniform(name, value, arrayIndex); };
    inline void SetVec3Uniform(const char* name, const Vector3& value, unsigned int arrayIndex) { m_shaderProgram->SetVec3Uniform(name, value, arrayIndex); };
    inline void SetFloatUniform(const char* name, float value, unsigned int arrayIndex) { m_shaderProgram->SetFloatUniform(name, value, arrayIndex); };
    inline void SetIntUniform(const char* name, int value, unsigned int arrayIndex) { m_shaderProgram->SetIntUniform(name, value, arrayIndex); };
    
    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    ShaderProgram* m_shaderProgram;
    RenderState m_renderState;

private:
    unsigned int m_samplerID;
    unsigned int m_diffuseID;
    unsigned int m_normalID;
    unsigned int m_emissiveID;
    unsigned int m_noiseID;
};
