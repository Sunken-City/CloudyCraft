#pragma once
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector2.hpp"
#include <vector>

class IBinaryWriter;
class IBinaryReader;
class AABB2;

class MeshBuilder
{
public:
    //ENUMS//////////////////////////////////////////////////////////////////////////
    enum MeshDataFlag
    {
        POSITION_BIT = 0,
        TANGENT_BIT,
        BITANGENT_BIT,
        NORMAL_BIT,
        COLOR_BIT,
        UV0_BIT,
        UV1_BIT,
        NORMALIZED_GLYPH_POSITION_BIT,
        NORMALIZED_STRING_POSITION_BIT,
        NORMALIZED_FRAG_POSITION_BIT,
        BONE_WEIGHTS_BIT,
        BONE_INDICES_BIT,
        FLOAT_DATA0_BIT,
        NUM_MESH_DATA
    };

    //TYPEDEFS//////////////////////////////////////////////////////////////////////////
    typedef Vector3(PatchFunction)(const void* userData, float x, float y);
    struct PlaneData
    {
        PlaneData() {};
        PlaneData(const Vector3& initialPosition, const Vector3& right, const Vector3& up) : initialPosition(initialPosition), right(right), up(up) {};
        Vector3 initialPosition;
        Vector3 right;
        Vector3 up;
    };

    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    MeshBuilder();
    ~MeshBuilder();

    //MEMBER FUNCTIONS//////////////////////////////////////////////////////////////////////////
    void Begin();
    void End();
    static MeshBuilder* Merge(MeshBuilder* meshBuilderArray, unsigned int numberOfMeshes);
    void CopyToMesh(Mesh* mesh, VertexCopyCallback* copyFunction, unsigned int sizeofVertex, Mesh::BindMeshToVAOForVertex* bindMeshFunction);
    void AddVertex(const Vector3& position);
    void AddIndex(int index);
    void AddLinearIndices();
    void AddQuadIndicesClockwise(unsigned int tlIndex, unsigned int trIndex, unsigned int blIndex, unsigned int brIndex);
    void AddQuadIndices(unsigned int tlIndex, unsigned int trIndex, unsigned int blIndex, unsigned int brIndex);
    void AddQuadIndices();
    void AddTexturedAABB(const AABB2& bounds, const Vector2& uvMins, const Vector2& uvMaxs, const RGBA& color);
    void AddText2D(const Vector2& position, const std::string& asciiText, float scale, const RGBA& tint = RGBA::WHITE, bool drawShadow = false, const BitmapFont* font = nullptr);
    void AddGlyph(const Vector3& bottomLeft, const Vector3& up, const Vector3& right, float upExtents, float rightExtents, const Vector2& uvMins, const Vector2& uvMaxs, const RGBA& color,
        float stringCoordXMin, float stringCoordXMax, float fragCoordXMin, float fragCoordXMax);
    void AddStringEffectFragment(const std::string& asciiText, const BitmapFont* font, float scale, float totalStringWidth, float totalWidthUpToNow,
        const Vector3& bottomLeft, const Vector3& up, const Vector3& right, float width, float height, int lineNum, float lineWidth, float lineAlignment);
    void AddCube(float sideLength, const RGBA& color = RGBA::WHITE);
    void AddUVSphere(float radius, int numSegments, const RGBA& color = RGBA::WHITE);
    void AddIcoSphere(float radius, const RGBA& color = RGBA::WHITE, int numPasses = 3, const Vector3& offset = Vector3::ZERO);
    void AddQuad(const Vector3& bottomLeft, const Vector3& up, float upLength, const Vector3& right, float rightLength, const RGBA& color = RGBA::WHITE, const Vector2& uvOffset = Vector2::ZERO, float uvStepSize = 1.0f);
    void AddLine(const Vector3& start, const Vector3& end, const RGBA& color = RGBA::WHITE, const Vector2& uvBegin  = Vector2::ZERO, const Vector2& uvEnd  = Vector2::ZERO);
    void BuildQuad(const Vector3& initialPosition, const Vector3& right, const Vector3& up, float startX, float endX, float startY, float endY, float startU = 0.0f, float endU = 1.0f, float startV = 0.0f, float endV = 1.0f);
    void BuildPlane(const Vector3& initialPosition, const Vector3& right, const Vector3& up, float startX, float endX, uint32_t xSections, float startY, float endY, uint32_t ySections);
    void BuildPlaneFromFunc(const Vector3& initialPosition, const Vector3& right, const Vector3& up, float startX, float endX, uint32_t xSections, float startY, float endY, uint32_t ySections);
    void BuildPatch(float startX, float endX, uint32_t xSections, float startY, float endY, uint32_t ySections, PatchFunction* patchFunction, void* userData);
    void FlipVs();

    //GETTERS//////////////////////////////////////////////////////////////////////////
    inline unsigned int GetCurrentIndex() { return m_vertices.size(); };

    //SETTERS//////////////////////////////////////////////////////////////////////////
    inline void SetColor(const RGBA& color) { m_stamp.color = color; SetMaskBit(COLOR_BIT); };
    inline void SetTangent(const Vector3& tangent) { m_stamp.tangent = tangent; SetMaskBit(TANGENT_BIT); };
    inline void SetBitangent(const Vector3& bitangent) { m_stamp.bitangent = bitangent; SetMaskBit(BITANGENT_BIT); };
    inline void SetNormal(const Vector3& normal) { m_stamp.normal = normal; SetMaskBit(NORMAL_BIT); };
    inline void SetUV(float u, float v) { m_stamp.uv0 = Vector2(u,v); SetMaskBit(UV0_BIT); };
    inline void SetUV(const Vector2& uv) { m_stamp.uv0 = uv; SetMaskBit(UV0_BIT); };
    inline void SetFloatData0(const Vector4& data) { m_stamp.floatData0 = data; SetMaskBit(FLOAT_DATA0_BIT); };

    //-----------------------------------------------------------------------------------
    inline void SetTBN(const Vector3& tangent, const Vector3& bitangent, const Vector3& normal)
    {
        m_stamp.tangent = tangent;
        m_stamp.bitangent = bitangent;
        m_stamp.normal = normal; 
        SetMaskBit(TANGENT_BIT); 
        SetMaskBit(BITANGENT_BIT); 
        SetMaskBit(NORMAL_BIT);
    };

    //-----------------------------------------------------------------------------------
    inline void SetBoneWeights(const Vector4Int& boneIndices, Vector4 boneWeights)
    {
        m_stamp.boneIndices = boneIndices;
        m_stamp.boneWeights = boneWeights;
        SetMaskBit(BONE_WEIGHTS_BIT);
        SetMaskBit(BONE_INDICES_BIT);
    };

    void ClearBoneWeights();
    bool IsSkinned() const;
    inline void SetNormalizedGlyphCoords(const Vector2& ngc) { m_stamp.normalizedGlyphPosition = ngc; };
    inline void SetNormalizedStringCoords(const Vector2& nsc) { m_stamp.normalizedStringPosition = nsc; }
    inline void SetNormalizedFragCoords(float nfc) { m_stamp.normalizedFragPosition = nfc; }
    inline void SetMaterialName(const char* materialName) { m_materialName = materialName; };
    inline void SetMaskBit(const MeshDataFlag flag) { m_dataMask |= (1 << flag); };
    inline void ClearMaskBit(const MeshDataFlag flag) { m_dataMask &= ~(1 << flag); };

    //QUERIES//////////////////////////////////////////////////////////////////////////
    inline bool IsInMask(const MeshDataFlag flag) { return ((m_dataMask & (1 << flag)) != 0); };

    //I/O//////////////////////////////////////////////////////////////////////////
    void WriteToFile(const char* filename);
    void WriteToStream(IBinaryWriter& writer);
    void ReadFromStream(IBinaryReader& reader);
    void ReadFromFile(const char* filename);
    void WriteDataMask(IBinaryWriter& writer);
    uint32_t ReadDataMask(IBinaryReader& reader);
    void RenormalizeSkinWeights();
    bool IsEmpty();
    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    std::vector<Vertex_Master> m_vertices;
    std::vector<unsigned int> m_indices;
    uint32_t m_dataMask;

private:
    //Tracks all info added to the mesh.
    Vertex_Master m_stamp;
    unsigned int m_startIndex;
    const char* m_materialName;
    Renderer::DrawMode m_drawMode;
    bool m_isSkinned;

    //1: Initial Version
    static const uint32_t FILE_VERSION = 1;
};