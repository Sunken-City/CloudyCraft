#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Input/Console.hpp"
#include "Engine/Input/BinaryWriter.hpp"
#include "Engine/Input/BinaryReader.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/MathUtils.hpp"

extern MeshBuilder* g_loadedMeshBuilder;
extern Mesh* g_loadedMesh;

//-----------------------------------------------------------------------------------
#if defined(TOOLS_BUILD)
CONSOLE_COMMAND(saveMesh)
{
    if (!args.HasArgs(1))
    {
        Console::instance->PrintLine("saveMesh <filename>", RGBA::RED);
        return;
    }
    std::string filename = args.GetStringArgument(0);
    if (!g_loadedMeshBuilder)
    {
        Console::instance->PrintLine("Error: No mesh has been loaded yet, use fbxLoad to bring in a mesh first.", RGBA::RED);
        return;
    }
    g_loadedMeshBuilder->WriteToFile(filename.c_str());
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(loadMesh)
{
    if (!args.HasArgs(1))
    {
        Console::instance->PrintLine("loadMesh <filename>", RGBA::RED);
        return;
    }
    std::string filename = args.GetStringArgument(0);
    if (g_loadedMeshBuilder)
    {
        delete g_loadedMeshBuilder;
    }
    g_loadedMeshBuilder = new MeshBuilder();
    g_loadedMeshBuilder->ReadFromFile(filename.c_str());
    g_loadedMesh = new Mesh();
    g_loadedMeshBuilder->CopyToMesh(g_loadedMesh, &Vertex_PCUTB::Copy, sizeof(Vertex_PCUTB), &Vertex_PCUTB::BindMeshToVAO);
}
#endif

//-----------------------------------------------------------------------------------
MeshBuilder::MeshBuilder()
    : m_startIndex(0)
    , m_materialName(nullptr)
    , m_dataMask(0)
    , m_drawMode(Renderer::DrawMode::TRIANGLES)
    , m_isSkinned(false)
    , m_stamp()
{

}

//-----------------------------------------------------------------------------------
MeshBuilder::~MeshBuilder()
{
    m_vertices.clear();
    m_indices.clear();
}

//-----------------------------------------------------------------------------------
void MeshBuilder::Begin()
{
    m_startIndex = m_vertices.size();
}

//-----------------------------------------------------------------------------------
void MeshBuilder::End()
{
    if (m_startIndex < m_vertices.size())
    {
        m_startIndex = m_vertices.size();
    }
}

//-----------------------------------------------------------------------------------
MeshBuilder* MeshBuilder::Merge(MeshBuilder* meshBuilderArray, unsigned int numberOfMeshes)
{
    MeshBuilder* combinedMesh = new MeshBuilder();
    for (unsigned int i = 0; i < numberOfMeshes; i++)
    {
        int numPreexistingVerts = combinedMesh->m_indices.size();
        MeshBuilder& currentMesh = meshBuilderArray[i];
        for (Vertex_Master vert : currentMesh.m_vertices)
        {
            combinedMesh->m_vertices.push_back(vert);
        }
        for (unsigned int index : currentMesh.m_indices)
        {
            combinedMesh->m_indices.push_back(index + (i * numPreexistingVerts));
        }
        combinedMesh->m_dataMask |= currentMesh.m_dataMask;
    }
    return combinedMesh;
}

//-----------------------------------------------------------------------------------
void MeshBuilder::CopyToMesh(Mesh* mesh, VertexCopyCallback* copyFunction, unsigned int sizeofVertex, Mesh::BindMeshToVAOForVertex* bindMeshFunction)
{
    // First, we need to allocate a buffer to copy 
    // our vertices into, that matches what the mesh
    // wants.  
    unsigned int vertexCount = m_vertices.size();
    if (vertexCount == 0) {
        // nothing in this mesh.
        return;
    }

    unsigned int vertexSize = sizeofVertex; //mesh->vdefn->vertexSize;
    unsigned int vertex_buffer_size = vertexCount * vertexSize;

    byte* vertexBuffer = new byte[vertex_buffer_size];
    byte* currentBufferIndex = vertexBuffer;

//	mesh->m_verts.clear();
    for (unsigned int vertex_index = 0;	vertex_index < vertexCount;	++vertex_index) 
    {
        copyFunction(m_vertices[vertex_index], currentBufferIndex);
        currentBufferIndex += vertexSize;
    }
    mesh->Init(vertexBuffer, vertexCount, sizeofVertex, m_indices.data(), m_indices.size(), bindMeshFunction);
    mesh->m_drawMode = this->m_drawMode;
    // Make sure we clean up after ourselves
    delete vertexBuffer;
}

//-----------------------------------------------------------------------------------
void MeshBuilder::AddVertex(const Vector3& position)
{
    m_stamp.position = position;
    m_vertices.push_back(m_stamp);
    SetMaskBit(POSITION_BIT);
}

//-----------------------------------------------------------------------------------
void MeshBuilder::AddQuadIndicesClockwise(unsigned int tlIndex, unsigned int trIndex, unsigned int blIndex, unsigned int brIndex)
{
    m_indices.push_back(brIndex);
    m_indices.push_back(blIndex);
    m_indices.push_back(tlIndex);
    m_indices.push_back(brIndex);
    m_indices.push_back(tlIndex);
    m_indices.push_back(trIndex);
}

//-----------------------------------------------------------------------------------
void MeshBuilder::AddQuadIndices(unsigned int tlIndex, unsigned int trIndex, unsigned int blIndex, unsigned int brIndex)
{
    m_indices.push_back(brIndex);
    m_indices.push_back(tlIndex);
    m_indices.push_back(blIndex);
    m_indices.push_back(brIndex);
    m_indices.push_back(trIndex);
    m_indices.push_back(tlIndex);
}

//-----------------------------------------------------------------------------------
void MeshBuilder::AddQuadIndices()
{
    unsigned int lastIndex = m_vertices.size(); 
    AddQuadIndices(lastIndex + 3, lastIndex + 2, lastIndex + 0, lastIndex + 1);
}

//-----------------------------------------------------------------------------------
void MeshBuilder::BuildQuad(
    const Vector3& initial_position,
    const Vector3& right,
    const Vector3& up,
    float startX, float endX,
    float startY, float endY,
    float startU, float endU,
    float startV, float endV)
{
    this->Begin();

    float const xStep = endX - startX;
    float const yStep = endY - startY;
    float const uStep = endU - startU;
    float const vStep = endV - startV;

    // So, what is our tangent, normal, and bitangent?
    Vector3 tangent = right;
    Vector3 bitangent = up;
    Vector3 normal = Vector3::Cross(bitangent, tangent);
    this->SetTBN(tangent, bitangent, normal);

    // Okay, now, let's add all our vertices
    float x, y;
    float u, v;

    y = startY;
    v = startV;

    for (unsigned int iy = 0; iy < 2; ++iy) {
        x = startX;
        u = startU;

        for (uint32_t ix = 0; ix < 2; ++ix) {
            this->SetUV(u, v);

            Vector3 position = initial_position
                + (right * x)
                + (up * y);

            this->AddVertex(position);

            x += xStep;
            u += uStep;
        }

        y += yStep;
        v += vStep;
    }

    this->AddQuadIndices(1, 3, 0, 2);

    this->End();
}

//-----------------------------------------------------------------------------------
void MeshBuilder::BuildPlane(
    const Vector3& initialPosition,
    const Vector3& right,
    const Vector3& up,
    float startX, float endX, uint32_t xSections,
    float startY, float endY, uint32_t ySections)
{
    ASSERT_OR_DIE(xSections > 0, "xSections passed in to BuildPlane has an invalid value");
    ASSERT_OR_DIE(ySections > 0, "ySections passed in to BuildPlane has an invalid value");

    this->Begin();

    //Add vertices to patch
    uint32_t xVertCount = xSections + 1;
    uint32_t yVertCount = ySections + 1;

    float const xRange = endX - startX;
    float const yRange = endY - startY;
    float const xStep = xRange / (float)xSections;
    float const yStep = yRange / (float)ySections;

    //Textures the entire patch
    float const uStep = 1.0f / (float)xSections;
    float const vStep = 1.0f / (float)ySections;

    // So, what is our tangent, normal, and bitangent?
    Vector3 tangent = right;
    Vector3 bitangent = up;
    Vector3 normal = Vector3::Cross(bitangent, tangent);
    this->SetTBN(tangent, bitangent, normal);

    unsigned int startVertexIndex = this->GetCurrentIndex();

    // Okay, now, let's add all our vertices
    float x, y;
    float u, v;

    y = startY;
    v = 0.0f;

    for (unsigned int iy = 0; iy < yVertCount; ++iy) {
        x = startX;
        u = 0.0f;

        for (uint32_t ix = 0; ix < xVertCount; ++ix) {
            this->SetUV(u, v);

            Vector3 position = initialPosition
                + (right * x)
                + (up * y);

            this->AddVertex(position);

            x += xStep;
            u += uStep;
        }

        y += yStep;
        v += vStep;
    }

    // Next, add all the indices for this patch
    for (unsigned int iy = 0; iy < ySections; ++iy) {
        for (unsigned int ix = 0; ix < xSections; ++ix) {

            unsigned int blIndex = startVertexIndex
                + (iy * xVertCount) + ix;
            unsigned int brIndex = blIndex + 1;
            unsigned int tlIndex = blIndex + xVertCount;
            unsigned int trIndex = tlIndex + 1;

            // How many indices is this actually adding
            // under the hood, it we're working with triangles?
            this->AddQuadIndices(tlIndex, trIndex, blIndex, brIndex);
        }
    }

    this->End();
}

//-----------------------------------------------------------------------------------
static Vector3 PlaneFunc(const void* userData, float x, float y)
{
    MeshBuilder::PlaneData const *plane = (MeshBuilder::PlaneData const*)userData;
    Vector3 position = plane->initialPosition
        + (plane->right * x)
        + (plane->up * y);
    position.z = sin(x + y);
    return position;
}

//-----------------------------------------------------------------------------------
void MeshBuilder::BuildPlaneFromFunc(
    const Vector3& initialPosition,
    const Vector3& right,
    const Vector3& up,
    float startX, float endX, uint32_t xSections,
    float startY, float endY, uint32_t ySections)
{
    MeshBuilder::PlaneData plane;
    plane.initialPosition = initialPosition;
    plane.right = right;
    plane.up = up;

    this->BuildPatch(
    startX, endX, xSections,
    startY, endY, ySections,
    PlaneFunc,
    &plane);
}

//-----------------------------------------------------------------------------------
void MeshBuilder::BuildPatch(
    float startX, float endX, uint32_t xSections,
    float startY, float endY, uint32_t ySections,
    PatchFunction* patchFunction,
    void *userData)
{
    ASSERT_OR_DIE(xSections > 0, "xSections passed in to BuildPlane has an invalid value");
    ASSERT_OR_DIE(ySections > 0, "ySections passed in to BuildPlane has an invalid value");

    this->Begin();

    uint32_t xVertCount = xSections + 1;
    uint32_t yVertCount = ySections + 1;

    float const xRange = endX - startX;
    float const yRange = endY - startY;
    float const xStep = xRange / (float)xSections;
    float const yStep = yRange / (float)ySections;

    float const uStep = 1.0f / (float)xSections;
    float const vStep = 1.0f / (float)ySections;

    uint32_t startVertIndex = this->GetCurrentIndex();

    //Add in the vertices
    float x, y;
    float u, v;

    y = startY;
    v = 0.0f;

    float const delta = .01f; // artitrarily small value, can go smaller

    for (uint32_t iy = 0; iy < yVertCount; ++iy) {
        x = startX;
        u = 0.0f;

        for (uint32_t ix = 0; ix < xVertCount; ++ix) {
            this->SetUV(u, v);

            // calculate tangent along u (that is, x)
            Vector3 tangent = patchFunction(userData, x + delta, y) - patchFunction(userData, x - delta, y);

            // calculate bitangent along v (taht is, y)
            Vector3 bitangent = patchFunction(userData, x, y + delta) - patchFunction(userData, x, y - delta);

            tangent.Normalize();
            bitangent.Normalize();
            Vector3 normal = Vector3::Cross(bitangent, tangent);
            bitangent = Vector3::Cross(tangent, normal);
            this->SetTBN(tangent, bitangent, normal);

            Vector3 position = patchFunction(userData, x, y);
            this->AddVertex(position);

            x += xStep;
            u += uStep;
        }

        y += yStep;
        v += vStep;
    }

    //Add all the indices for the patch
    for (uint32_t iy = 0; iy < ySections; ++iy) {
        for (uint32_t ix = 0; ix < xSections; ++ix) {

            uint32_t blIdx = startVertIndex
                + (iy * xVertCount) + ix;
            uint32_t brIdx = blIdx + 1;
            uint32_t tlIdx = blIdx + xVertCount;
            uint32_t trIdx = tlIdx + 1;

            this->AddQuadIndices(tlIdx, trIdx, blIdx, brIdx);
        }
    }
    this->End();
}

//-----------------------------------------------------------------------------------
void MeshBuilder::WriteDataMask(IBinaryWriter& writer)
{
    if ((m_dataMask & (1 << POSITION_BIT)) != 0)
    {
        writer.WriteString("Position");
    }
    if ((m_dataMask & (1 << TANGENT_BIT)) != 0)
    {
        writer.WriteString("Tangent");
    }
    if ((m_dataMask & (1 << BITANGENT_BIT)) != 0)
    {
        writer.WriteString("Bitangent");
    }
    if ((m_dataMask & (1 << NORMAL_BIT)) != 0)
    {
        writer.WriteString("Normal");
    }
    if ((m_dataMask & (1 << COLOR_BIT)) != 0)
    {
        writer.WriteString("Color");
    }
    if ((m_dataMask & (1 << UV0_BIT)) != 0)
    {
        writer.WriteString("UV0");
    }
    if ((m_dataMask & (1 << UV1_BIT)) != 0)
    {
        writer.WriteString("UV1");
    }
    if ((m_dataMask & (1 << NORMALIZED_GLYPH_POSITION_BIT)) != 0)
    {
        writer.WriteString("NormalizedGlyphPosition");
    }
    if ((m_dataMask & (1 << NORMALIZED_STRING_POSITION_BIT)) != 0)
    {
        writer.WriteString("NormalizedStringPosition");
    }
    if ((m_dataMask & (1 << NORMALIZED_FRAG_POSITION_BIT)) != 0)
    {
        writer.WriteString("NormalizedFragPosition");
    }
    if ((m_dataMask & (1 << BONE_INDICES_BIT)) != 0)
    {
        writer.WriteString("BoneIndices");
    }
    if ((m_dataMask & (1 << BONE_WEIGHTS_BIT)) != 0)
    {
        writer.WriteString("BoneWeights");
    }
    if ((m_dataMask & (1 << FLOAT_DATA0_BIT)) != 0)
    {
        writer.WriteString("FloatData0");
    }

    writer.WriteString(nullptr);
}

//-----------------------------------------------------------------------------------
uint32_t MeshBuilder::ReadDataMask(IBinaryReader& reader)
{
    uint32_t mask = 0;
    const char* str = nullptr;
    size_t size = reader.ReadString(str, 64);
    while (size > 0) 
    {
        if (strcmp(str, "Position") == 0)
        {
            mask |= (1 << POSITION_BIT);
        }
        else if (strcmp(str, "Tangent") == 0)
        {
            mask |= (1 << TANGENT_BIT);
        }
        else if (strcmp(str, "Bitangent") == 0)
        {
            mask |= (1 << BITANGENT_BIT);
        }
        else if (strcmp(str, "Normal") == 0)
        {
            mask |= (1 << NORMAL_BIT);
        }
        else if (strcmp(str, "Color") == 0)
        {
            mask |= (1 << COLOR_BIT);
        }
        else if (strcmp(str, "UV0") == 0)
        {
            mask |= (1 << UV0_BIT);
        }
        else if (strcmp(str, "UV1") == 0)
        {
            mask |= (1 << UV1_BIT);
        }
        else if (strcmp(str, "NormalizedGlyphPosition") == 0)
        {
            mask |= (1 << NORMALIZED_GLYPH_POSITION_BIT);
        }
        else if (strcmp(str, "NormalizedStringPosition") == 0)
        {
            mask |= (1 << NORMALIZED_STRING_POSITION_BIT);
        }
        else if (strcmp(str, "NormalizedFragPosition") == 0)
        {
            mask |= (1 << NORMALIZED_FRAG_POSITION_BIT);
        }
        else if (strcmp(str, "BoneIndices") == 0)
        {
            mask |= (1 << BONE_INDICES_BIT);
        }
        else if (strcmp(str, "BoneWeights") == 0)
        {
            mask |= (1 << BONE_WEIGHTS_BIT);
        }
        else if (strcmp(str, "FloatData0") == 0)
        {
            mask |= (1 << FLOAT_DATA0_BIT);
        }
        size = reader.ReadString(str, 64);
    }
    delete str;
    return mask;
}

//-----------------------------------------------------------------------------------
void MeshBuilder::RenormalizeSkinWeights()
{
    //Renormalize the bone weights.
    float totalWeight = m_stamp.boneWeights.x + m_stamp.boneWeights.y + m_stamp.boneWeights.z + m_stamp.boneWeights.w;
    if (totalWeight > 0.0f)
    {
        m_stamp.boneWeights /= totalWeight;
    }
    else
    {
        m_stamp.boneWeights = { 1.0f, 0.0f, 0.0f, 0.0f };
    }
}

bool MeshBuilder::IsEmpty()
{
    return m_vertices.size() == 0;
}

//-----------------------------------------------------------------------------------
void MeshBuilder::AddTexturedAABB(const AABB2& bounds, const Vector2& uvMins, const Vector2& uvMaxs, const RGBA& color)
{
    int startingVertex = m_vertices.size();
    SetColor(color);
    SetUV(uvMins);
    AddVertex(Vector3(bounds.mins.x, bounds.mins.y, 0.0f));
    SetUV(Vector2(uvMaxs.x, uvMins.y));
    AddVertex(Vector3(bounds.maxs.x, bounds.mins.y, 0.0f));
    SetUV(uvMaxs);
    AddVertex(Vector3(bounds.maxs.x, bounds.maxs.y, 0.0f));
    SetUV(Vector2(uvMins.x, uvMaxs.y));
    AddVertex(Vector3(bounds.mins.x, bounds.maxs.y, 0.0f));
    AddQuadIndices(startingVertex + 3, startingVertex + 2, startingVertex + 0, startingVertex + 1);
}

//-----------------------------------------------------------------------------------
void MeshBuilder::AddText2D(const Vector2& position, const std::string& asciiText, float scale, const RGBA& tint /*= RGBA::WHITE*/, bool drawShadow /*= false*/, const BitmapFont* font /*= nullptr*/)
{
    if (asciiText.empty())
    {
        return;
    }
    if (font == nullptr)
    {
        font = Renderer::instance->m_defaultFont;
    }
    int stringLength = asciiText.size();
    Vector2 cursorPosition = position + (Vector2::UNIT_Y * (float)font->m_maxHeight * scale);
    const Glyph* previousGlyph = nullptr;
    for (int i = 0; i < stringLength; i++)
    {
        unsigned char currentCharacter = asciiText[i];
        const Glyph* glyph = font->GetGlyph(currentCharacter);
        float glyphWidth = static_cast<float>(glyph->width) * scale;
        float glyphHeight = static_cast<float>(glyph->height) * scale;

        if (previousGlyph)
        {
            const Vector2 kerning = font->GetKerning(*previousGlyph, *glyph);
            cursorPosition += (kerning * scale);
        }
        Vector2 offset = Vector2(glyph->xOffset * scale, -glyph->yOffset * scale);
        Vector2 topRight = cursorPosition + offset + Vector2(glyphWidth, 0.0f);
        Vector2 bottomLeft = cursorPosition + offset - Vector2(0.0f, glyphHeight);
        AABB2 quadBounds = AABB2(bottomLeft, topRight);
        AABB2 glyphBounds = font->GetTexCoordsForGlyph(*glyph);
        if (drawShadow)
        {
            float shadowWidthOffset = glyphWidth / 10.0f;
            float shadowHeightOffset = glyphHeight / -10.0f;
            Vector2 shadowOffset = Vector2(shadowWidthOffset, shadowHeightOffset);
            AABB2 shadowBounds = AABB2(bottomLeft + shadowOffset, topRight + shadowOffset);
            this->AddTexturedAABB(shadowBounds, glyphBounds.mins, glyphBounds.maxs, RGBA::BLACK);
        }
        this->AddTexturedAABB(quadBounds, glyphBounds.mins, glyphBounds.maxs, tint);
        cursorPosition.x += glyph->xAdvance * scale;
        previousGlyph = glyph;
    }
}

//-----------------------------------------------------------------------------------
void MeshBuilder::AddGlyph(const Vector3& bottomLeft, const Vector3& up, const Vector3& right, float upExtents, float rightExtents, const Vector2& uvMins, const Vector2& uvMaxs, const RGBA& color,
    float stringCoordXMin, float stringCoordXMax, float fragCoordXMin, float fragCoordXMax)
{
    int startingVertex = m_vertices.size();
    Vector3 topLeft = bottomLeft + (up * upExtents);
    Vector3 bottomRight = bottomLeft + (right * rightExtents);
    Vector3 topRight = topLeft + (right * rightExtents);
    SetColor(color);
    SetUV(uvMins);
    SetNormalizedGlyphCoords(Vector2::ZERO);
    SetNormalizedStringCoords(Vector2(stringCoordXMin, 0.f));
    SetNormalizedFragCoords(fragCoordXMin);
    AddVertex(bottomLeft);
    SetUV(Vector2(uvMaxs.x, uvMins.y));
    SetNormalizedGlyphCoords(Vector2::UNIT_X);
    SetNormalizedStringCoords(Vector2(stringCoordXMax, 0.f));
    SetNormalizedFragCoords(fragCoordXMax);
    AddVertex(bottomRight);
    SetUV(uvMaxs);
    SetNormalizedGlyphCoords(Vector2::ONE);
    SetNormalizedStringCoords(Vector2(stringCoordXMax, 0.f));
    SetNormalizedFragCoords(fragCoordXMax);
    AddVertex(topRight);
    SetUV(Vector2(uvMins.x, uvMaxs.y));
    SetNormalizedGlyphCoords(Vector2::UNIT_Y);
    SetNormalizedStringCoords(Vector2(stringCoordXMin, 0.f));
    SetNormalizedFragCoords(fragCoordXMin);
    AddVertex(topLeft);
    AddQuadIndices(startingVertex + 3, startingVertex + 2, startingVertex + 0, startingVertex + 1);
}

//-----------------------------------------------------------------------------------
void MeshBuilder::AddStringEffectFragment(const std::string& asciiText, const BitmapFont* font, float scale, float totalStringWidth, float totalWidthUpToNow,
    const Vector3& bottomLeft, const Vector3& up, const Vector3& right, float width, float height, int lineNum, float lineWidth, float lineAlignment)
{
    if (asciiText.empty())
    {
        return;
    }
    if (font == nullptr)
    {
        font = Renderer::instance->m_defaultFont;
    }
    int stringLength = asciiText.size();
    Vector3 cursorPosition = bottomLeft + (up * height) - ((float)lineNum * font->m_maxHeight * scale * up) + ((width - lineWidth) * lineAlignment * right * scale) + (totalWidthUpToNow * scale * right);
    const Glyph* previousGlyph = nullptr;
    float totalWidthSoFar = totalWidthUpToNow;
    float localWidthSoFar = 0;
    float fragmentWidth = font->CalcTextWidth(asciiText, scale);
    for (int i = 0; i < stringLength; i++)
    {
        unsigned char currentCharacter = asciiText[i];
        const Glyph* glyph = font->GetGlyph(currentCharacter);
        float glyphWidth = static_cast<float>(glyph->width) * scale;
        float glyphHeight = static_cast<float>(glyph->height) * scale;

        if (previousGlyph)
        {
            const Vector2 kerning = font->GetKerning(*previousGlyph, *glyph);
            cursorPosition += kerning.x * scale * right + kerning.y * scale * up;
        }
        Vector3 offset = (right * (glyph->xOffset * scale)) + (up * (-glyph->yOffset * scale));
        Vector3 topRight = cursorPosition + offset + (right * (glyphWidth));
        Vector3 bl = cursorPosition + offset - (up * glyphHeight);
        AABB2 quadBounds = AABB2(Vector2(bl.x, bl.y), Vector2(topRight.x, topRight.y));
        AABB2 glyphBounds = font->GetTexCoordsForGlyph(*glyph);
        // 		if (drawShadow)
        // 		{
        // 			float shadowWidthOffset = glyphWidth / 10.0f;
        // 			float shadowHeightOffset = glyphHeight / -10.0f;
        // 			Vector3 shadowOffset = (right * shadowWidthOffset) + (up * shadowHeightOffset);
        // 			//Vector2 shadowOffset = Vector2(shadowWidthOffset, shadowHeightOffset);
        // 			//AABB2 shadowBounds = AABB2(bottomLeft + shadowOffset, topRight + shadowOffset);
        // 			this->AddGlyph(bottomLeft, up, right, shadowHeightOffset, shadowWidthOffset, glyphBounds.mins, glyphBounds.maxs, RGBA::BLACK);
        // 		}
        //this->AddTexturedAABB(quadBounds, glyphBounds.mins, glyphBounds.maxs, RGBA::WHITE);
        float stringXMin = totalWidthSoFar / totalStringWidth;
        float fragXMin = localWidthSoFar / fragmentWidth;
        totalWidthSoFar += glyph->xAdvance * scale;
        localWidthSoFar += glyph->xAdvance * scale;
        float stringXMax = totalWidthSoFar / totalStringWidth;
        float fragXMax = localWidthSoFar / fragmentWidth;
        cursorPosition += glyph->xAdvance * scale * right;
        this->AddGlyph(bl, up, right, (glyph->height * scale), (glyph->width * scale), glyphBounds.mins, glyphBounds.maxs, RGBA::WHITE, stringXMin, stringXMax, fragXMin, fragXMax);
        previousGlyph = glyph;
    }
}

//-----------------------------------------------------------------------------------
void MeshBuilder::AddIndex(int index)
{
    m_indices.push_back(index);
}

//-----------------------------------------------------------------------------------
void MeshBuilder::AddLinearIndices()
{
    for (unsigned int i = 0; i < m_vertices.size(); ++i)
    {
        AddIndex(i);
    }
}

//-----------------------------------------------------------------------------------
void MeshBuilder::WriteToFile(const char* filename)
{
    BinaryFileWriter writer;
    ASSERT_OR_DIE(writer.Open(filename), "File Open failed!");
    {
        WriteToStream(writer);
    }
    writer.Close();
}

//-----------------------------------------------------------------------------------
void MeshBuilder::WriteToStream(IBinaryWriter& writer)
{
    //FILE VERSION
    //material id
    //vertex data mask (ie: position, tangent, normal, etc...)
    //vertices
    //indices

    writer.Write<uint32_t>(FILE_VERSION);
    writer.WriteString(m_materialName);
    WriteDataMask(writer);
    uint32_t vertexCount = m_vertices.size();
    uint32_t indicesCount = m_indices.size();
    writer.Write<uint32_t>(vertexCount);
    for (Vertex_Master vertex : m_vertices)
    {
        //TODO("Clean this up when you're not running on no sleep, it's not efficient");
        IsInMask(POSITION_BIT) ? writer.Write<Vector3>(vertex.position) : false;
        IsInMask(TANGENT_BIT) ? writer.Write<Vector3>(vertex.tangent) : false;
        IsInMask(BITANGENT_BIT) ? writer.Write<Vector3>(vertex.bitangent) : false;
        IsInMask(NORMAL_BIT) ? writer.Write<Vector3>(vertex.normal) : false;
        IsInMask(COLOR_BIT) ? writer.Write<RGBA>(vertex.color) : false;
        IsInMask(UV0_BIT) ? writer.Write<Vector2>(vertex.uv0) : false;
        IsInMask(UV1_BIT) ? writer.Write<Vector2>(vertex.uv1) : false;
        IsInMask(BONE_INDICES_BIT) ? writer.Write<Vector4Int>(vertex.boneIndices) : false;
        IsInMask(BONE_WEIGHTS_BIT) ? writer.Write<Vector4>(vertex.boneWeights) : false;
        IsInMask(FLOAT_DATA0_BIT) ? writer.Write<Vector4>(vertex.floatData0) : false;
    }
    writer.Write<uint32_t>(indicesCount);
    for (unsigned int index : m_indices)
    {
        writer.Write<unsigned int>(index);
    }
}

//-----------------------------------------------------------------------------------
void MeshBuilder::ReadFromStream(IBinaryReader& reader)
{
    //FILE VERSION
    //material id
    //vertex data mask (ie: position, tangent, normal, etc...)
    //vertices
    //indices

    uint32_t fileVersion;
    const char* materialName = nullptr;
    uint32_t vertexCount;
    uint32_t indicesCount;

    ASSERT_OR_DIE(reader.Read<uint32_t>(fileVersion), "Failed to read file version");
    reader.ReadString(materialName, 64);
    SetMaterialName(materialName);
    m_dataMask = ReadDataMask(reader);
    ASSERT_OR_DIE(reader.Read<uint32_t>(vertexCount), "Failed to read vertex count");
    for (unsigned int i = 0; i < vertexCount; ++i)
    {
        //TODO("Clean this up when you're not running on no sleep");
        //TODO("Sleep so that you can actually clean this up");
        Vertex_Master vertex;
        IsInMask(POSITION_BIT) ? reader.Read<Vector3>(vertex.position) : false;
        IsInMask(TANGENT_BIT) ? reader.Read<Vector3>(vertex.tangent) : false;
        IsInMask(BITANGENT_BIT) ? reader.Read<Vector3>(vertex.bitangent) : false;
        IsInMask(NORMAL_BIT) ? reader.Read<Vector3>(vertex.normal) : false;
        IsInMask(COLOR_BIT) ? reader.Read<RGBA>(vertex.color) : false;
        IsInMask(UV0_BIT) ? reader.Read<Vector2>(vertex.uv0) : false;
        IsInMask(UV1_BIT) ? reader.Read<Vector2>(vertex.uv1) : false;
        IsInMask(BONE_INDICES_BIT) ? reader.Read<Vector4Int>(vertex.boneIndices) : false;
        IsInMask(BONE_WEIGHTS_BIT) ? reader.Read<Vector4>(vertex.boneWeights) : false;
        IsInMask(FLOAT_DATA0_BIT) ? reader.Read<Vector4>(vertex.floatData0) : false;
        m_vertices.push_back(vertex);
    }	
    ASSERT_OR_DIE(reader.Read<uint32_t>(indicesCount), "Failed to read index count");
    for (unsigned int i = 0; i < indicesCount; ++i)
    {
        unsigned int index;
        reader.Read<unsigned int>(index);
        m_indices.push_back(index);
    }
}

//-----------------------------------------------------------------------------------
void MeshBuilder::ReadFromFile(const char* filename)
{
    BinaryFileReader reader;
    ASSERT_OR_DIE(reader.Open(filename), "File Open failed!");
    {
        ReadFromStream(reader);
    }
    reader.Close();
}

//-----------------------------------------------------------------------------------
void MeshBuilder::FlipVs()
{
    for (unsigned int index = 0; index < m_vertices.size(); ++index)
    {
        m_vertices[index].uv0.y = 1.0f - m_vertices[index].uv0.y;
    }
}

//-----------------------------------------------------------------------------------
void MeshBuilder::ClearBoneWeights()
{
    m_stamp.boneIndices = Vector4Int::ZERO;
    m_stamp.boneWeights = Vector4::UNIT_X;
    ClearMaskBit(BONE_WEIGHTS_BIT);
    ClearMaskBit(BONE_INDICES_BIT);
}

//-----------------------------------------------------------------------------------
bool MeshBuilder::IsSkinned() const
{
    return m_isSkinned;
}

//-----------------------------------------------------------------------------------
void MeshBuilder::AddCube(float sideLength, const RGBA& color /*= RGBA::WHITE*/)
{
    const float halfSideLength = sideLength / 2.0f;
  
    AddQuad(Vector3::UP * sideLength, Vector3::FORWARD, sideLength, Vector3::RIGHT, sideLength, color, Vector2::ZERO, 1.0f);									//TOP
    AddQuad(Vector3::FORWARD * sideLength, -Vector3::FORWARD, sideLength, Vector3::RIGHT, sideLength, color, Vector2::ZERO, 1.0f);								//BOTTOM
    AddQuad(Vector3::ZERO, Vector3::UP, sideLength, Vector3::RIGHT, sideLength, color, Vector2::ZERO, 1.0f);													//SOUTH
    AddQuad(Vector3::FORWARD * sideLength + Vector3::RIGHT * sideLength, Vector3::UP, sideLength, -Vector3::RIGHT, sideLength, color, Vector2::ZERO, 1.0f);		//NORTH
    AddQuad(Vector3::FORWARD * sideLength, Vector3::UP, sideLength, -Vector3::FORWARD, sideLength, color, Vector2::ZERO, 1.0f);									//WEST
    AddQuad(Vector3::RIGHT * sideLength, Vector3::UP, sideLength, Vector3::FORWARD, sideLength, color, Vector2::ZERO, 1.0f);									//EAST

}

//-----------------------------------------------------------------------------------
//Based off of code from http://stackoverflow.com/a/13846592/2619871
void MeshBuilder::AddUVSphere(float radius, int numSegments, const RGBA& color /*= RGBA::WHITE*/)
{
    const float radiansPerSide = MathUtils::HALF_PI / static_cast<float>(numSegments);
    float const normalizedPhiStepSize = 1.0f / (float)(numSegments - 1);
    float const normalizedThetaStepSize = 1.0f / (float)(numSegments - 1);

    this->Begin();
    SetColor(color);

    for (int currentLatitude = 0; currentLatitude < numSegments; ++currentLatitude) 
    {
        for (int currentLongitude = 0; currentLongitude < numSegments; ++currentLongitude) 
        {
            const float stepPhi = (-MathUtils::HALF_PI) + (MathUtils::PI * currentLatitude * normalizedPhiStepSize);
            const float stepTheta = MathUtils::TWO_PI * currentLongitude * normalizedThetaStepSize;
            const float y = sin(stepPhi);
            const float tangentY = 0.0f;
            const float bitangentY = cos(stepPhi);

            const float x = cos(stepTheta) * cos(stepPhi);
            const float tangentX = -sin(stepTheta) * cos(stepPhi);
            const float bitangentX = cos(stepTheta) * -sin(stepPhi);

            const float z = sin(stepTheta) * cos(stepPhi);
            const float tangentZ = cos(stepTheta) * cos(stepPhi);
            const float bitangentZ = sin(stepTheta) * -sin(stepPhi);

            SetUV(Vector2(currentLongitude * normalizedThetaStepSize, 1.0f - currentLatitude * normalizedPhiStepSize));
            SetTangent(Vector3(tangentX, tangentY, tangentZ));
            SetBitangent(Vector3(bitangentX, bitangentY, bitangentZ));

            AddVertex(Vector3(x, y, z) * radius);

            int currentRow = currentLatitude * numSegments;
            int nextRow = (currentLatitude + 1) * numSegments;

            AddIndex(nextRow + (currentLongitude + 1));
            AddIndex(nextRow + currentLongitude);
            AddIndex(currentRow + currentLongitude);

            AddIndex(currentRow + (currentLongitude + 1));
            AddIndex(nextRow + (currentLongitude + 1));
            AddIndex(currentRow + currentLongitude);
        }
    }
}

//-----------------------------------------------------------------------------------
void MeshBuilder::AddIcoSphere(float radius, const RGBA& color /*= RGBA::WHITE*/, int numPasses /*= 3*/, const Vector3& offset /*= Vector3::ZERO*/)
{
    Vector3 initialPoints[6] = { { 0, 0, radius },{ 0, 0, -radius },{ -radius, -radius, 0 },{ radius, -radius, 0 },{ radius, radius, 0 },{ -radius,  radius, 0 } };
    Vector2 initialUVs[6] = { {0.5f, 0.5f}, {0.5f, 0.5f}, {1.0f, 1.0f}, {0.0f, 1.0f}, { 0.0f, 0.0f },{ 1.0f, 0.0f } };
    SetColor(color);
    const int initialIndex = m_vertices.size();

    for (int i = 0; i < 6; i++)
    {
        SetUV(initialUVs[i]);
        AddVertex((Vector3::GetNormalized(initialPoints[i]) * radius) + offset);
    }

    AddIndex(0 + initialIndex); AddIndex(3 + initialIndex); AddIndex(4 + initialIndex);
    AddIndex(0 + initialIndex); AddIndex(4 + initialIndex); AddIndex(5 + initialIndex);
    AddIndex(0 + initialIndex); AddIndex(5 + initialIndex); AddIndex(2 + initialIndex);
    AddIndex(0 + initialIndex); AddIndex(2 + initialIndex); AddIndex(3 + initialIndex);
    AddIndex(1 + initialIndex); AddIndex(4 + initialIndex); AddIndex(3 + initialIndex);
    AddIndex(1 + initialIndex); AddIndex(5 + initialIndex); AddIndex(4 + initialIndex);
    AddIndex(1 + initialIndex); AddIndex(2 + initialIndex); AddIndex(5 + initialIndex);
    AddIndex(1 + initialIndex); AddIndex(3 + initialIndex); AddIndex(2 + initialIndex);

    for (int i = 0; i < numPasses; i++)
    {
        int numberOfFaces = m_indices.size() / 3;
        int indicesIndex = initialIndex;
        for (int j = 0; j < numberOfFaces; j++)
        {
            //Get first 3 indices
            const int x = indicesIndex++;
            const int y = indicesIndex++;
            const int z = indicesIndex++;
            // Calculate the midpoints 
            Vector3 point1 = Vector3::GetMidpoint(m_vertices.at(m_indices.at(x)).position, m_vertices.at(m_indices.at(y)).position);
            Vector3 point2 = Vector3::GetMidpoint(m_vertices.at(m_indices.at(y)).position, m_vertices.at(m_indices.at(z)).position);
            Vector3 point3 = Vector3::GetMidpoint(m_vertices.at(m_indices.at(z)).position, m_vertices.at(m_indices.at(x)).position);

            //Calculate UV midpoints
            Vector2 uvPoint1 = Vector2::GetMidpoint(m_vertices.at(m_indices.at(x)).uv0, m_vertices.at(m_indices.at(y)).uv0);
            Vector2 uvPoint2 = Vector2::GetMidpoint(m_vertices.at(m_indices.at(y)).uv0, m_vertices.at(m_indices.at(z)).uv0);
            Vector2 uvPoint3 = Vector2::GetMidpoint(m_vertices.at(m_indices.at(z)).uv0, m_vertices.at(m_indices.at(x)).uv0);

            //Move the points to the outside of our sphere.
            point1.Normalize();
            point2.Normalize();
            point3.Normalize();

            //Add these vertices to the list of verts, and store their array location.
            //Naiive way to check for dupes is here. I'll find a better solution later.
            int point1Location = -1;
            int point2Location = -1;
            int point3Location = -1;
            for (unsigned int q = 0; q < m_vertices.size(); q++)
            {
                Vector3 compare = m_vertices.at(q).position;
                if (compare == point1)
                    point1Location = q;
                if (compare == point2)
                    point2Location = q;
                if (compare == point3)
                    point3Location = q;
            }
            if (point1Location == -1)
            {
                point1Location = m_vertices.size();
                SetUV(uvPoint1);
                AddVertex(point1 * radius);
            }

            if (point2Location == -1)
            {
                point2Location = m_vertices.size();
                SetUV(uvPoint2);
                AddVertex(point2 * radius);
            }

            if (point3Location == -1)
            {
                point3Location = m_vertices.size();
                SetUV(uvPoint3);
                AddVertex(point3 * radius);
            }

            //Create 3 new faces (the outer triangles, the pieces of the triforce)
            AddIndex(m_indices.at(x));		AddIndex(point1Location);		AddIndex(point3Location);
            AddIndex(point1Location);		AddIndex(m_indices.at(y));		AddIndex(point2Location);
            AddIndex(point3Location);		AddIndex(point2Location);		AddIndex(m_indices.at(z));

            //Replace the original face with the inner, upside-down triangle (not the triforce)
            m_indices.at(x) = point1Location;
            m_indices.at(y) = point2Location;
            m_indices.at(z) = point3Location;
        }
    }
}

//----------------------------------------------------------------------------------
void MeshBuilder::AddQuad(
    const Vector3& bottomLeft, 
    const Vector3& up, 
    float upLength, 
    const Vector3& right, 
    float rightLength, 
    const RGBA& color /*= RGBA::WHITE*/, 
    const Vector2& uvOffset /*= Vector2::ZERO*/, 
    float uvStepSize /*= 1.0f*/
    )
{
    unsigned int currentVert = this->m_vertices.size();
    SetColor(color);

    SetUV(uvOffset + (Vector2::UNIT_Y * uvStepSize));
    SetTangent(right);
    SetBitangent(up);
    AddVertex(bottomLeft);

    SetUV(uvOffset);
    SetTangent(right);
    SetBitangent(up);
    AddVertex(bottomLeft + (up * upLength));

    SetUV(uvOffset + (Vector2::ONE * uvStepSize));
    SetTangent(right);
    SetBitangent(up);
    AddVertex(bottomLeft + (right * rightLength));

    SetUV(uvOffset + (Vector2::UNIT_X * uvStepSize));
    SetTangent(right);
    SetBitangent(up);
    AddVertex(bottomLeft + (up * upLength) + (right * rightLength));
        
    AddIndex(2 + currentVert);
    AddIndex(1 + currentVert);
    AddIndex(0 + currentVert);
    AddIndex(2 + currentVert);
    AddIndex(3 + currentVert);
    AddIndex(1 + currentVert);
}

void MeshBuilder::AddLine(const Vector3& start, const Vector3& end, const RGBA& color/* = RGBA::WHITE*/, const Vector2& uvBegin /* = Vector2::ZERO*/, const Vector2& uvEnd /* = Vector2::ZERO*/)
{
    uint32_t currentVert = m_vertices.size();
    m_drawMode = Renderer::DrawMode::LINES;
    SetColor(color);
    SetUV(uvBegin);
    AddVertex(start);
    SetUV(uvEnd);
    AddVertex(end);
    AddIndex(0 + currentVert);
    AddIndex(1 + currentVert);
}