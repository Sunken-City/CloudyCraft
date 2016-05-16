#include "Engine/Renderer/Skeleton.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Input/Console.hpp"

Skeleton* g_loadedSkeleton = nullptr;

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(saveSkel)
{
    if (!args.HasArgs(1))
    {
        Console::instance->PrintLine("saveSkel <filename>", RGBA::RED);
        return;
    }
    std::string filename = args.GetStringArgument(0);
    if (!g_loadedSkeleton)
    {
        Console::instance->PrintLine("Error: No skeleton has been loaded yet, use fbxLoad to bring in a mesh with a skeleton first.", RGBA::RED);
        return;
    }
    g_loadedSkeleton->WriteToFile(filename.c_str());
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(loadSkel)
{
    if (!args.HasArgs(1))
    {
        Console::instance->PrintLine("loadSkel <filename>", RGBA::RED);
        return;
    }
    std::string filename = args.GetStringArgument(0);
    if (g_loadedSkeleton)
    {
        delete g_loadedSkeleton;
    }
    g_loadedSkeleton = new Skeleton();
    g_loadedSkeleton->ReadFromFile(filename.c_str());
}

//-----------------------------------------------------------------------------------
Skeleton::~Skeleton()
{
    delete m_joints->m_mesh;
    delete m_joints->m_material;
    delete m_joints;
    delete m_bones->m_mesh;
    delete m_bones->m_material;
    delete m_bones;
}

//-----------------------------------------------------------------------------------
void Skeleton::AddJoint(const char* str, int parentJointIndex, Matrix4x4 initialBoneToModelMatrix)
{
    m_names.push_back(std::string(str));
    m_parentIndices.push_back(parentJointIndex);
    m_boneToModelSpace.push_back(initialBoneToModelMatrix);
    Matrix4x4 modelToBoneMatrix = initialBoneToModelMatrix;
    Matrix4x4::MatrixInvert(&modelToBoneMatrix);
    m_modelToBoneSpace.push_back(modelToBoneMatrix);
}

//-----------------------------------------------------------------------------------
int Skeleton::FindJointIndex(const std::string& name)
{
    for (unsigned int i = 0; i < m_names.size(); ++i)
    {
        if (m_names[i] == name)
        {
            return i;
        }
    }
    return INVALID_JOINT_INDEX;
}

//-----------------------------------------------------------------------------------
Joint Skeleton::GetJoint(int index)
{
    return Joint(m_names[index], m_parentIndices[index], m_modelToBoneSpace[index], m_boneToModelSpace[index]);
}

//-----------------------------------------------------------------------------------
void Skeleton::Render() const
{
    if (!m_joints)
    {
        MeshBuilder builder;
        for (const Matrix4x4& modelSpaceMatrix : m_boneToModelSpace)
        {
            builder.AddIcoSphere(1.0f, RGBA::BLUE, 0, modelSpaceMatrix.GetTranslation());
        }
        m_joints = new MeshRenderer(new Mesh(), new Material(new ShaderProgram("Data/Shaders/fixedVertexFormat.vert", "Data/Shaders/fixedVertexFormat.frag"), 
            RenderState(RenderState::DepthTestingMode::OFF, RenderState::FaceCullingMode::RENDER_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)));
        m_joints->m_material->SetDiffuseTexture(Renderer::instance->m_defaultTexture);
        builder.CopyToMesh(m_joints->m_mesh, &Vertex_PCUTB::Copy, sizeof(Vertex_PCUTB), &Vertex_PCUTB::BindMeshToVAO);
    }
    if (!m_bones)
    {
        MeshBuilder builder;
        for (unsigned int i = 0; i < m_parentIndices.size(); i++)
        {
            int parentIndex = m_parentIndices[i];
            if (parentIndex >= 0)
            {
                builder.AddLine(m_boneToModelSpace[i].GetTranslation(), m_boneToModelSpace[parentIndex].GetTranslation(), RGBA::SEA_GREEN);
            }
        }
        m_bones = new MeshRenderer(new Mesh(), new Material(new ShaderProgram("Data/Shaders/fixedVertexFormat.vert", "Data/Shaders/fixedVertexFormat.frag"),
            RenderState(RenderState::DepthTestingMode::OFF, RenderState::FaceCullingMode::RENDER_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)));
        m_bones->m_material->SetDiffuseTexture(Renderer::instance->m_defaultTexture);
        builder.CopyToMesh(m_bones->m_mesh, &Vertex_PCUTB::Copy, sizeof(Vertex_PCUTB), &Vertex_PCUTB::BindMeshToVAO);
    }
    m_joints->Render();
    m_bones->Render();
}

//-----------------------------------------------------------------------------------
uint32_t Skeleton::GetJointCount()
{
    return m_names.size();
}

//-----------------------------------------------------------------------------------
Joint::Joint(const std::string& name, int parentIndex, const Matrix4x4& modelToBoneSpace, const Matrix4x4& boneToModelSpace)
    : m_name(name)
    , m_parentIndex(parentIndex)
    , m_modelToBoneSpace(modelToBoneSpace)
    , m_boneToModelSpace(boneToModelSpace)
{

}

//-----------------------------------------------------------------------------------
void Skeleton::WriteToFile(const char* filename)
{
    BinaryFileWriter writer;
    ASSERT_OR_DIE(writer.Open(filename), "File Open failed!");
    {
        WriteToStream(writer);
    }
    writer.Close();
}

//-----------------------------------------------------------------------------------
void Skeleton::WriteToStream(IBinaryWriter& writer)
{
    //FILE VERSION
    //Number of joints
    //Joint Names
    //Joint Heirarchy
    //Initial Model Space

    writer.Write<uint32_t>(FILE_VERSION);
    writer.Write<uint32_t>(m_names.size());
    
    for (const std::string& str : m_names)
    {
        writer.WriteString(str.c_str());
    }
    for (int index : m_parentIndices)
    {
        writer.Write<int>(index);
    }
    for (const Matrix4x4& mat : m_boneToModelSpace)
    {
        //writer.WriteBytes(mat.data, 16);
        for (int i = 0; i < 16; ++i)
        {
            writer.Write<float>(mat.data[i]);
        }
    }
}

//-----------------------------------------------------------------------------------
void Skeleton::ReadFromStream(IBinaryReader& reader)
{
    //FILE VERSION
    //Number of joints
    //Joint Names
    //Joint Heirarchy
    //Initial Model Space

    uint32_t fileVersion = 0;
    ASSERT_OR_DIE(reader.Read<uint32_t>(fileVersion), "Failed to read file version");
    ASSERT_OR_DIE(fileVersion == FILE_VERSION, "File version didn't match!");
    uint32_t numberOfJoints = 0;
    ASSERT_OR_DIE(reader.Read<uint32_t>(numberOfJoints), "Failed to read number of joints");

    for (unsigned int i = 0; i < numberOfJoints; ++i)
    {
        const char* jointName = nullptr;
        reader.ReadString(jointName, 64);
        m_names.push_back(jointName);
    }
    for (unsigned int i = 0; i < numberOfJoints; ++i)
    {
        int index = 0;
        reader.Read<int>(index);
        m_parentIndices.push_back(index);
    }
    for (unsigned int i = 0; i < numberOfJoints; ++i)
    {
        Matrix4x4 matrix = Matrix4x4::IDENTITY;
        for (int j = 0; j < 16; ++j)
        {
            reader.Read<float>(matrix.data[j]);
        }
        m_boneToModelSpace.push_back(matrix);
        //m_boneToModelSpace.push_back(Matrix4x4((float*)reader.ReadBytes(16)));
        //Matrix4x4 invertedMatrix = m_boneToModelSpace[i];
        //Matrix4x4::MatrixInvert(&invertedMatrix);
        //m_modelToBoneSpace.push_back(invertedMatrix);
    }
}

//-----------------------------------------------------------------------------------
void Skeleton::ReadFromFile(const char* filename)
{
    BinaryFileReader reader;
    ASSERT_OR_DIE(reader.Open(filename), "File Open failed!");
    {
        ReadFromStream(reader);
    }
    reader.Close();
}