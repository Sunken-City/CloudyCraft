#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include "Engine/Renderer/OpenGLExtensions.hpp"

//-----------------------------------------------------------------------------------
RenderState::RenderState(DepthTestingMode depthTesting, FaceCullingMode faceCulling, BlendMode blendMode)
    : depthTestingMode(depthTesting)
    , faceCullingMode(faceCulling)
    , blendMode(blendMode)
{

}

//-----------------------------------------------------------------------------------
RenderState::~RenderState()
{
}

//-----------------------------------------------------------------------------------
void RenderState::SetState() const
{
    switch (depthTestingMode)
    {
    case DepthTestingMode::OFF:
        Renderer::instance->EnableDepthTest(false);
        break;
    case DepthTestingMode::ON:
        Renderer::instance->EnableDepthTest(true);
        break;
    case DepthTestingMode::DISABLE_WRITE:
        Renderer::instance->DisableDepthWrite();
        break;
    default:
        break;
    }
    switch (faceCullingMode)
    {
    case FaceCullingMode::CULL_BACK_FACES:
        Renderer::instance->EnableFaceCulling(true);
        break;
    case FaceCullingMode::RENDER_BACK_FACES:
        Renderer::instance->EnableFaceCulling(false);
        break;
    default:
        break;
    }
    switch (blendMode)
    {
    case BlendMode::ADDITIVE_BLEND:
        Renderer::instance->EnableAdditiveBlending();
        break;
    case BlendMode::ALPHA_BLEND:
        Renderer::instance->EnableAlphaBlending();
        break;
    case BlendMode::INVERTED_BLEND:
        Renderer::instance->EnableInvertedBlending();
        break;
    default:
        break;
    }
}

//-----------------------------------------------------------------------------------
void RenderState::ClearState() const
{
    //Default state of the renderer.
    Renderer::instance->EnableDepthTest(true);
    Renderer::instance->EnableFaceCulling(true);
    Renderer::instance->EnableAlphaBlending();
    Renderer::instance->EnableDepthWrite();
}

//-----------------------------------------------------------------------------------
Material::Material(ShaderProgram* program, const RenderState& renderState) 
    : m_shaderProgram(program)
    , m_samplerID(Renderer::instance->CreateSampler(GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT))
    , m_renderState(renderState)
{

}

//-----------------------------------------------------------------------------------
Material::~Material()
{
    Renderer::instance->DeleteSampler(m_samplerID);
}

//-----------------------------------------------------------------------------------
void Material::SetMatrices(const Matrix4x4& model, const Matrix4x4& view, const Matrix4x4& projection)
{
    m_shaderProgram->SetMatrix4x4Uniform("gModel", model);
    m_shaderProgram->SetMatrix4x4Uniform("gView", view);
    m_shaderProgram->SetMatrix4x4Uniform("gProj", projection);
}

//-----------------------------------------------------------------------------------
void Material::BindAvailableTextures() const
{
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, m_diffuseID);
    glBindSampler(0, m_samplerID);
    m_shaderProgram->SetIntUniform("gDiffuseTexture", 0);

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, m_normalID);
    glBindSampler(1, m_samplerID);
    m_shaderProgram->SetIntUniform("gNormalTexture", 1);

    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, m_emissiveID);
    glBindSampler(2, m_samplerID);
    m_shaderProgram->SetIntUniform("gEmissiveTexture", 2);

    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, m_noiseID);
    glBindSampler(3, m_samplerID);
    m_shaderProgram->SetIntUniform("gNoiseTexture", 3);
}

//-----------------------------------------------------------------------------------
void Material::UnbindAvailableTextures() const
{
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, NULL);
    glBindSampler(0, NULL);

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, NULL);
    glBindSampler(0, NULL);

    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, NULL);
    glBindSampler(0, NULL);

    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, NULL);
    glBindSampler(0, NULL);

    glActiveTexture(GL_TEXTURE0);
}

//-----------------------------------------------------------------------------------
void Material::SetDiffuseTexture(Texture* texture)
{
    m_diffuseID = texture->m_openglTextureID;
}

//-----------------------------------------------------------------------------------
void Material::SetNormalTexture(Texture* texture)
{
    m_normalID = texture->m_openglTextureID;
}

//-----------------------------------------------------------------------------------
void Material::SetEmissiveTexture(Texture* texture)
{
    m_emissiveID = texture->m_openglTextureID;
}

//-----------------------------------------------------------------------------------
void Material::SetNoiseTexture(Texture* texture)
{
    m_noiseID = texture->m_openglTextureID;
}

//-----------------------------------------------------------------------------------
void Material::SetDiffuseTexture(const std::string& diffusePath)
{
    SetTexture("gDiffuseTexture", Texture::CreateOrGetTexture(diffusePath)->m_openglTextureID);
    m_diffuseID = Texture::CreateOrGetTexture(diffusePath)->m_openglTextureID;
}

//-----------------------------------------------------------------------------------
void Material::SetNormalTexture(const std::string& normalPath)
{
    SetTexture("gNormalTexture", Texture::CreateOrGetTexture(normalPath)->m_openglTextureID);
    m_normalID = Texture::CreateOrGetTexture(normalPath)->m_openglTextureID;
}

//-----------------------------------------------------------------------------------
void Material::SetEmissiveTexture(const std::string& emissivePath)
{
    SetTexture("gEmissiveTexture", Texture::CreateOrGetTexture(emissivePath)->m_openglTextureID);
    m_emissiveID = Texture::CreateOrGetTexture(emissivePath)->m_openglTextureID;
}

//-----------------------------------------------------------------------------------
void Material::SetNoiseTexture(const std::string& noisePath)
{
    SetTexture("gNoiseTexture", Texture::CreateOrGetTexture(noisePath)->m_openglTextureID);
    m_noiseID = Texture::CreateOrGetTexture(noisePath)->m_openglTextureID;
}

//-----------------------------------------------------------------------------------
void Material::SetTexture(const char*, unsigned int)
{
}

//-----------------------------------------------------------------------------------
void Material::SetUpRenderState() const
{
    m_renderState.SetState();
    glUseProgram(m_shaderProgram->m_shaderProgramID);
}

//-----------------------------------------------------------------------------------
void Material::CleanUpRenderState() const
{
    m_renderState.ClearState();
    glUseProgram(NULL);
}
