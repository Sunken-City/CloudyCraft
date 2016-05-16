#include "Engine/Tools/fbx.hpp"
#include "Engine/Input/Console.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MatrixStack4x4.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/Skeleton.hpp"
#include "Engine/Renderer/AnimationMotion.hpp"

Mesh* g_loadedMesh = nullptr;
MeshBuilder* g_loadedMeshBuilder = nullptr;
extern Skeleton* g_loadedSkeleton;
extern AnimationMotion* g_loadedMotion;

#if defined(TOOLS_BUILD)
    //For tools only
    #include "ThirdParty/FBX/include/fbxsdk.h"
#include "../Math/Vector4Int.hpp"
    #pragma comment(lib, "libfbxsdk-md.lib")

    //-----------------------------------------------------------------------------------
    CONSOLE_COMMAND(fbxList)
    {
        std::string filename = args.GetStringArgument(0);
        FbxListScene(filename.c_str());
    }

    //-----------------------------------------------------------------------------------
    CONSOLE_COMMAND(fbxLoad)
    {
        if (!(args.HasArgs(2) || args.HasArgs(1)))
        {
            Console::instance->PrintLine("fbxLoad <file path> <scale>", RGBA::RED);
            return;
        }
        std::string filename = args.GetStringArgument(0);

        float scale = args.HasArgs(2) ? args.GetFloatArgument(1) : 1.0f;
        Matrix4x4 transform;
        Matrix4x4::MatrixMakeScale(&transform, scale);
        
        SceneImport* import = FbxLoadSceneFromFile(filename.c_str(), Matrix4x4::IDENTITY, false, transform);
        if (import == nullptr)
        {
            Console::instance->PrintLine(Stringf("Failed to load file. '%s'", filename.c_str()));
            DebuggerPrintf("Failed to load file. '%s'", filename.c_str());
        }
        else
        {
            Console::instance->PrintLine(Stringf("Loaded '%s'. Had %i meshes.", filename.c_str(), import->meshes.size()));
            DebuggerPrintf("Loaded '%s'. Had %i meshes.", filename.c_str(), import->meshes.size());
            g_loadedMesh = new Mesh();
            g_loadedMeshBuilder = MeshBuilder::Merge(import->meshes.data(), import->meshes.size());
            g_loadedMeshBuilder->AddLinearIndices();
            g_loadedMeshBuilder->CopyToMesh(g_loadedMesh, &Vertex_SkinnedPCTN::Copy, sizeof(Vertex_SkinnedPCTN), &Vertex_SkinnedPCTN::BindMeshToVAO);
            g_loadedSkeleton = import->skeletons.size() > 0 ? import->skeletons[0] : nullptr;
            g_loadedMotion = import->motions.size() > 0 ? import->motions[0] : nullptr;
        }
        delete import;
    }

    //-----------------------------------------------------------------------------------
    struct SkinWeight
    {
        SkinWeight() : indices(Vector4Int::ZERO), weights(Vector4::ZERO) {};
        Vector4Int indices;
        Vector4 weights;
    };

    //-----------------------------------------------------------------------------------
    static Vector3 ToEngineVec3(const FbxVector4& fbxVec4)
    {
        return Vector3((float)fbxVec4.mData[0],
            (float)fbxVec4.mData[1],
            (float)fbxVec4.mData[2]);
    }

    //-----------------------------------------------------------------------------------
    static Vector4 ToEngineVec4(const FbxDouble4& fbxVec)
    {
        return Vector4(
            static_cast<float>(fbxVec.mData[0]),
            static_cast<float>(fbxVec.mData[1]),
            static_cast<float>(fbxVec.mData[2]),
            static_cast<float>(fbxVec.mData[3])
            );
    }

    //-----------------------------------------------------------------------------------
    static RGBA ToEngineRGBA(const FbxColor& fbxColor)
    {
        return RGBA(
            static_cast<float>(fbxColor.mRed),
            static_cast<float>(fbxColor.mBlue),
            static_cast<float>(fbxColor.mGreen),
            static_cast<float>(fbxColor.mAlpha)
            );
    }

    //-----------------------------------------------------------------------------------
    static Matrix4x4 ToEngineMatrix(const FbxMatrix& fbxMat)
    {
        Matrix4x4 mat;
        Matrix4x4::MatrixSetRows(&mat,
            ToEngineVec4(fbxMat.mData[0]),
            ToEngineVec4(fbxMat.mData[1]),
            ToEngineVec4(fbxMat.mData[2]),
            ToEngineVec4(fbxMat.mData[3]));
        return mat;
    }

    //-----------------------------------------------------------------------------------
    static const char* GetAttributeTypeName(FbxNodeAttribute::EType type)
    {
        switch (type) 
        { 
            case FbxNodeAttribute::eUnknown: return "unidentified";            
            case FbxNodeAttribute::eNull: return "null";            
            case FbxNodeAttribute::eMarker: return "marker";           
            case FbxNodeAttribute::eSkeleton: return "skeleton";            
            case FbxNodeAttribute::eMesh: return "mesh";            
            case FbxNodeAttribute::eNurbs: return "nurbs";          
            case FbxNodeAttribute::ePatch: return "patch";          
            case FbxNodeAttribute::eCamera: return "camera";          
            case FbxNodeAttribute::eCameraStereo: return "stereo";  
            case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";  
            case FbxNodeAttribute::eLight: return "light";         
            case FbxNodeAttribute::eOpticalReference: return "optical reference";   
            case FbxNodeAttribute::eOpticalMarker: return "marker";          
            case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";        
            case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";         
            case FbxNodeAttribute::eBoundary: return "boundary";          
            case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";        
            case FbxNodeAttribute::eShape: return "shape";           
            case FbxNodeAttribute::eLODGroup: return "lodgroup";       
            case FbxNodeAttribute::eSubDiv: return "subdiv";            
            default: return "unknown"; 
        }
    }

    //-----------------------------------------------------------------------------------
    static void PrintAttribute(FbxNodeAttribute* attribute, int depth)
    {
        if (attribute == nullptr)
        {
            return;
        }

        FbxNodeAttribute::EType type = attribute->GetAttributeType();
        const char* typeName = GetAttributeTypeName(type);
        const char* attribName = attribute->GetName();

        Console::instance->PrintLine(Stringf("%*s- type='%s', name='%s'\n", depth, " ", typeName, attribName));
        DebuggerPrintf("%*s- type='%s', name='%s'\n", depth, " ", typeName, attribName);
    }

    //-----------------------------------------------------------------------------------
    static void PrintNode(FbxNode* node, int depth)
    {
        Console::instance->PrintLine(Stringf("%*sNode [%s]\n", depth, " ", node->GetName()));
        DebuggerPrintf("%*sNode [%s]\n", depth, " ", node->GetName());
        for (int i = 0; i < node->GetNodeAttributeCount(); ++i)
        {
            PrintAttribute(node->GetNodeAttributeByIndex(i), depth);
        }
        for (int i = 0; i < node->GetChildCount(); ++i)
        {
            PrintNode(node->GetChild(i), depth + 1);
        }
    }

    //-----------------------------------------------------------------------------------
    void FbxListScene(const char* filename)
    {
        FbxManager* fbxManager = FbxManager::Create();
        if (nullptr == fbxManager)
        {
            Console::instance->PrintLine("Could not create fbx manager.");
            DebuggerPrintf("Could not create fbx manager.");
            return;
        }
        
        FbxIOSettings* ioSettings = FbxIOSettings::Create(fbxManager, IOSROOT); //Name of object is blank, we don't care
        fbxManager->SetIOSettings(ioSettings);

        //Create an importer
        FbxImporter* importer = FbxImporter::Create(fbxManager, "");

        bool loadSuccessful = importer->Initialize(filename, -1, fbxManager->GetIOSettings());
        if (loadSuccessful)
        {
            //We have imported the FBX
            FbxScene* scene = FbxScene::Create(fbxManager, "");
            bool importSuccessful = importer->Import(scene);
            if (importSuccessful)
            {
                FbxNode* root = scene->GetRootNode();
                PrintNode(root, 0);
            }

            FBX_SAFE_DESTROY(scene);
        }
        else
        {
            Console::instance->PrintLine(Stringf("Could not import scene: %s", filename));
            DebuggerPrintf("Could not import scene: %s", filename);
        }

        FBX_SAFE_DESTROY(importer);
        FBX_SAFE_DESTROY(ioSettings);
        FBX_SAFE_DESTROY(fbxManager);
    }

    //-----------------------------------------------------------------------------------
    static bool GetPosition(Vector3* outPosition, const Matrix4x4& transform, FbxMesh* mesh, int polyIndex, int vertIndex)
    {
        FbxVector4 fbxPosition;
        int controlIndex = mesh->GetPolygonVertex(polyIndex, vertIndex);
        fbxPosition = mesh->GetControlPointAt(controlIndex);
        *outPosition = Vector3(Vector4(ToEngineVec3(fbxPosition), 1.0f) * transform);
        return true;
    }

    //-----------------------------------------------------------------------------------
    template <typename ElemType, typename VarType>
    static bool GetObjectFromElement(FbxMesh* mesh, int polyIndex, int vertIndex, ElemType* elem, VarType* outVar)
    {
        if (nullptr == elem)
        {
            return false;
        }
        switch(elem->GetMappingMode())
        {
            case FbxGeometryElement::eByControlPoint:
            {
                int controlIndex = mesh->GetPolygonVertex(polyIndex, vertIndex);
                switch (elem->GetReferenceMode())
                {
                case FbxGeometryElement::eDirect:
                    if (controlIndex < elem->GetDirectArray().GetCount())
                    {
                        *outVar = elem->GetDirectArray().GetAt(controlIndex);
                        return true;
                    }
                    break;
                case FbxGeometryElement::eIndexToDirect:
                    if (controlIndex < elem->GetIndexArray().GetCount())
                    {
                        int index = elem->GetIndexArray().GetAt(controlIndex);
                        *outVar = elem->GetDirectArray().GetAt(index);
                        return true;
                    }
                    break;
                default:
                    break;
                }
            }
            break;
            case FbxGeometryElement::eByPolygonVertex:
            {
                int directVertexIndex = (polyIndex * 3) + vertIndex;
                switch (elem->GetReferenceMode())
                {
                case FbxGeometryElement::eDirect:
                    if (directVertexIndex < elem->GetDirectArray().GetCount())
                    {
                        *outVar = elem->GetDirectArray().GetAt(directVertexIndex);
                        return true;
                    }
                    break;
                case FbxGeometryElement::eIndexToDirect:
                    if (directVertexIndex < elem->GetIndexArray().GetCount())
                    {
                        int index = elem->GetIndexArray().GetAt(directVertexIndex);
                        *outVar = elem->GetDirectArray().GetAt(index);
                        return true;
                    }
                    break;
                default:
                    break;
                }
            }
            break;
            default:
                ERROR_AND_DIE("Undefined Mapping Mode")
                break;
        }
        return false;
    }

    //-----------------------------------------------------------------------------------
    static bool GetNormal(Vector3& outNormal, const Matrix4x4& transform, FbxMesh* mesh, int polyIndex, int vertIndex)
    {
        FbxVector4 normal;
        FbxGeometryElementNormal* normals = mesh->GetElementNormal(0);
        if (GetObjectFromElement(mesh, polyIndex, vertIndex, normals, &normal))
        {
            Vector3 n = ToEngineVec3(normal);
            outNormal = Vector3(Vector4(n, 0.0f) * transform);
            return true;
        }

        return false;
    }

    //-----------------------------------------------------------------------------------
    static bool GetUV(Vector2& outUV, FbxMesh* mesh, int polyIndex, int vertIndex, int uvIndex)
    {
        FbxVector2 uv;
        FbxGeometryElementUV* uvs = mesh->GetElementUV(uvIndex);
        if (GetObjectFromElement(mesh, polyIndex, vertIndex, uvs, &uv))
        {
            outUV = Vector2((float)uv.mData[0], (float)uv.mData[1]);
            return true;
        }

        return false;
    }

    //-----------------------------------------------------------------------------------
    static bool GetColor(RGBA& outColor, FbxMesh* mesh, int polyIndex, int vertIndex)
    {
        FbxColor color;
        FbxGeometryElementVertexColor* colors = mesh->GetElementVertexColor(0);
        if (GetObjectFromElement(mesh, polyIndex, vertIndex, colors, &color))
        {
            outColor = ToEngineRGBA(color);
            return true;
        }

        return false;
    }

    //-----------------------------------------------------------------------------------
    static void ImportVertex(MeshBuilder& builder, const Matrix4x4& transform, FbxMesh* mesh, int polyIndex, int vertIndex, std::vector<SkinWeight>& skinWeights)
    {
        Vector3 normal;
        if (GetNormal(normal, transform, mesh, polyIndex, vertIndex))
        {
            builder.SetNormal(normal);

            //Temporary hack to get around not supporting multiple vertex definitions
            Vector3 bitangent = Vector3::UP;
            if (normal == bitangent)
            {
                bitangent = Vector3::FORWARD;
            }
            Vector3 tangent = Vector3::Cross(bitangent, normal);
            bitangent = Vector3::Cross(normal, tangent);

            builder.SetTangent(tangent);
            builder.SetBitangent(bitangent);
        }
        
        RGBA color;
        if (GetColor(color, mesh, polyIndex, vertIndex))
        {
            builder.SetColor(color);
        }

        Vector2 uv;
        if (GetUV(uv, mesh, polyIndex, vertIndex, 0))
        {
            builder.SetUV(uv);
        }

        //Set Skin Weights
        int controlIndex = mesh->GetPolygonVertex(polyIndex, vertIndex);
        if (controlIndex < skinWeights.size())
        {
            builder.SetBoneWeights(skinWeights[controlIndex].indices, skinWeights[controlIndex].weights);
            builder.RenormalizeSkinWeights(); //Just to be safe.
        }
        else
        {
            builder.ClearBoneWeights();
        }

        Vector3 position;
        if (GetPosition(&position, transform, mesh, polyIndex, vertIndex))
        {
            builder.AddVertex(position);
        }
    }

    //-----------------------------------------------------------------------------------
    static Matrix4x4 GetGeometricTransform(FbxMesh* mesh)
    {
        return Matrix4x4::IDENTITY;
    }

    //-----------------------------------------------------------------------------------
    static bool HasSkinWeights(const FbxMesh* mesh)
    {
        int deformerCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
        return (deformerCount > 0);
    }

    //-----------------------------------------------------------------------------------
    static void AddHighestWeightWhileKickingTheLowestWeightOut(SkinWeight* skinWeight, int jointIndex, float weight)
    {
        //GO THROUGH YOUR SKIN WEIGHTS
        //FIND THE LOWEST (lower than this) AND KICK IT OUT OF THE LIST
        //Replace the lowest value with what was sent in, if you are higher than the lower value.
        float lowestWeight = weight;
        int lowestIndex = jointIndex;
        int spotForLowest = 0;
        if (skinWeight->weights.x < lowestWeight)
        {
            lowestWeight = skinWeight->weights.x;
            lowestIndex = skinWeight->indices.x;
            spotForLowest = 1;
        }
        if (skinWeight->weights.y < lowestWeight)
        {
            lowestWeight = skinWeight->weights.y;
            lowestIndex = skinWeight->indices.y;
            spotForLowest = 2;
        }
        if (skinWeight->weights.z < lowestWeight)
        {
            lowestWeight = skinWeight->weights.z;
            lowestIndex = skinWeight->indices.z;
            spotForLowest = 3;
        }
        if (skinWeight->weights.w < lowestWeight)
        {
            lowestWeight = skinWeight->weights.w;
            lowestIndex = skinWeight->indices.w;
            spotForLowest = 4;
        }
        if (spotForLowest == 0)
        {
            return;
        }
        else if (spotForLowest == 1)
        {
            skinWeight->weights.x = weight;
            skinWeight->indices.x = jointIndex;
        }
        else if (spotForLowest == 2)
        {
            skinWeight->weights.y = weight;
            skinWeight->indices.y = jointIndex;
        }
        else if (spotForLowest == 3)
        {
            skinWeight->weights.z = weight;
            skinWeight->indices.z = jointIndex;
        }
        else if (spotForLowest == 4)
        {
            skinWeight->weights.w = weight;
            skinWeight->indices.w = jointIndex;
        }
    }

    //-----------------------------------------------------------------------------------
    static void GetSkinWeights(SceneImport* import, std::vector<SkinWeight>& skinWeights, const FbxMesh* mesh, std::map<int, FbxNode*>& nodeToJointIndex)
    {
        int controlPointCount = mesh->GetControlPointsCount();
        skinWeights.resize(controlPointCount);
        for (size_t i = 0; i < skinWeights.size(); ++i) 
        {
            skinWeights[i].indices = Vector4Int(0, 0, 0, 0);
            skinWeights[i].weights = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
        }
        int deformerCount = mesh->GetDeformerCount((FbxDeformer::eSkin));
        ASSERT_OR_DIE(deformerCount == 1, "Deformer count wasn't 1");

        for (int deformerIndex = 0; deformerIndex < deformerCount; ++deformerIndex)
        {
            FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(deformerIndex, FbxDeformer::eSkin);

            if (skin == nullptr)
            {
                continue;
            }

            //Clusters are a link between this skin object, bones, and the verts that bone affects.
            int clusterCount = skin->GetClusterCount();
            for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
            {
                FbxCluster* cluster = skin->GetCluster(clusterIndex);
                const FbxNode* linkNode = cluster->GetLink();

                //Not associated with a bone? Ignore it, we don't care about it
                if (linkNode == nullptr)
                {
                    continue;
                }

                int jointIndex = Skeleton::INVALID_JOINT_INDEX;
                for (auto iter = nodeToJointIndex.begin(); iter != nodeToJointIndex.end(); ++iter)
                {
                    if (iter->second == linkNode)
                    {
                        jointIndex = iter->first;
                        break;
                    }
                }

                if (jointIndex == Skeleton::INVALID_JOINT_INDEX)
                {
                    continue;
                }

                int* controlPointIndices = cluster->GetControlPointIndices();
                int indexCount = cluster->GetControlPointIndicesCount();
                double* weights = cluster->GetControlPointWeights();

                for (int i = 0; i < indexCount; ++i)
                {
                    int controlIndex = controlPointIndices[i];
                    double weight = weights[i];

                    SkinWeight* skinWeight = &skinWeights[controlIndex];
                    AddHighestWeightWhileKickingTheLowestWeightOut(skinWeight, jointIndex, (float)weight);
                }
            }
        }

        for (SkinWeight& sw : skinWeights)
        {
            //Renormalize all the skin weights
            //Be sure to set weights such that all weights add up to 1
            //All weights should add up to 1, so things that were all zeroes add up to 1
            //If skin-weights were never added, make sure you set it to 1, 0, 0, 0
            float totalWeight = sw.weights.x + sw.weights.y + sw.weights.z + sw.weights.w;
            if(totalWeight > 0.0f)
            {
                sw.weights /= totalWeight;
            }
            else
            {
                sw.weights = { 1.0f, 0.0f, 0.0f, 0.0f };
            }
        }
    }

    //THIS MUST HAPPEN AFTER IMPORTING SKELETONS.
    //-----------------------------------------------------------------------------------
    static void ImportMesh(SceneImport* import, FbxMesh* mesh, MatrixStack4x4& matrixStack, std::map<int, FbxNode*>& nodeToJointIndex)
    {
        MeshBuilder builder = MeshBuilder();
        ASSERT_OR_DIE(mesh->IsTriangleMesh(), "Was unable to load the mesh, it's not a triangle mesh!");
        Matrix4x4 geoTransform = GetGeometricTransform(mesh);
        matrixStack.Push(geoTransform);

        int controlPointCount = mesh->GetControlPointsCount();

        //Figure out our weighs for all verts before importing any of them
        std::vector<SkinWeight> skinWeights;
        if (HasSkinWeights(mesh))
        {
            skinWeights.resize(controlPointCount);
            GetSkinWeights(import, skinWeights, mesh, nodeToJointIndex);
        }
        else
        {
            FbxNode* node = mesh->GetNode();
            //Walk tree up till you reach the node associated with that joint.
            //Find the first parent node that has a joint associated with it
            //All vertices (fully weighted)
            //All Skin Weights = indices{jointINdex, 0, 0, 0 } weights{1.0f, 0.0f, 0.0f, 0.0f};

            int jointIndex = Skeleton::INVALID_JOINT_INDEX;
            for (auto iter = nodeToJointIndex.begin(); iter != nodeToJointIndex.end(); ++iter)
            {
                if (iter->second == node)
                {
                    jointIndex = iter->first;
                    break;
                }
            }

            if (jointIndex == Skeleton::INVALID_JOINT_INDEX)
            {
                for (unsigned int i = 0; i < skinWeights.size(); ++i)
                {
                    skinWeights[i].indices = Vector4Int::ZERO;
                    skinWeights[i].weights = Vector4::UNIT_X;
                }
            }
            else
            {
                for (unsigned int i = 0; i < skinWeights.size(); ++i)
                {
                    skinWeights[i].indices = Vector4Int(jointIndex, 0, 0, 0);
                    skinWeights[i].weights = Vector4::UNIT_X;
                }
            }
        }

        builder.Begin();
        {
            Matrix4x4 transform = matrixStack.GetTop();
            int polyCount = mesh->GetPolygonCount();
            for (int polyIndex = 0; polyIndex < polyCount; ++polyIndex)
            {
                int vertCount = mesh->GetPolygonSize(polyIndex);
                ASSERT_OR_DIE(vertCount == 3, "Vertex count was not 3");
                for (int vertIndex = 0; vertIndex < vertCount; ++vertIndex)
                {
                    ImportVertex(builder, transform, mesh, polyIndex, vertIndex, skinWeights);
                }
            }
        }
        builder.End();

        FbxSurfaceMaterial* material = mesh->GetNode()->GetMaterial(0);
        builder.SetMaterialName(material->GetName());

        matrixStack.Pop();

        import->meshes.push_back(builder);
    }

    //-----------------------------------------------------------------------------------
    static void ImportNodeAttribute(SceneImport* import, FbxNodeAttribute* attrib, MatrixStack4x4& matrixStack, std::map<int, FbxNode*>& nodeToJointIndex)
    {
        if (attrib == nullptr)
        {
            return;
        }

        switch (attrib->GetAttributeType())
        {
        case FbxNodeAttribute::eMesh:
            ImportMesh(import, (FbxMesh*)attrib, matrixStack, nodeToJointIndex);
            break;

        default:
            break;
        }
    }

    //-----------------------------------------------------------------------------------
    static Matrix4x4 GetNodeTransform(FbxNode* node)
    {
        FbxMatrix fbxLocalMatrix = node->EvaluateLocalTransform();
        return ToEngineMatrix(fbxLocalMatrix);
    }

    //-----------------------------------------------------------------------------------
    static void ImportSceneNode(SceneImport* import, FbxNode* node, MatrixStack4x4& matrixStack, std::map<int, FbxNode*>& nodeToJointIndex)
    {
        if (node == nullptr)
        {
            return;
        }

        Matrix4x4 nodeLocalTransform = GetNodeTransform(node);
        matrixStack.Push(nodeLocalTransform);

        //New stuff! We want to load it!
        int attributeCount = node->GetNodeAttributeCount();
        for (int attributeIndex = 0; attributeIndex < attributeCount; ++attributeIndex)
        {
            ImportNodeAttribute(import, node->GetNodeAttributeByIndex(attributeIndex), matrixStack, nodeToJointIndex);
        }

        //Import Children
        int childCount = node->GetChildCount();
        for (int childIndex = 0; childIndex < childCount; ++childIndex)
        {
            ImportSceneNode(import, node->GetChild(childIndex), matrixStack, nodeToJointIndex);
        }

        matrixStack.Pop();
    }

    //-----------------------------------------------------------------------------------
    static void TriangulateScene(FbxScene* scene)
    {
        FbxGeometryConverter converter(scene->GetFbxManager());
        converter.Triangulate(scene, true); //True if we should replace the node, false if we want to make new nodes
    }

    //-----------------------------------------------------------------------------------
    static Matrix4x4 GetGeometricTransform(FbxNode* node)
    {
        Matrix4x4 returnMatrix = Matrix4x4::IDENTITY;
        if ((node != nullptr) && (node->GetNodeAttribute() != nullptr))
        {
            const FbxVector4 geoTrans = node->GetGeometricTranslation(FbxNode::eSourcePivot);
            const FbxVector4 geoRot = node->GetGeometricRotation(FbxNode::eSourcePivot);
            const FbxVector4 geoScale = node->GetGeometricScaling(FbxNode::eSourcePivot);

            FbxMatrix geoMat;
            geoMat.SetTRS(geoTrans, geoRot, geoScale);

            returnMatrix = ToEngineMatrix(geoMat);
        }

        return returnMatrix;
    }

    //-----------------------------------------------------------------------------------
    static Skeleton* ImportSkeleton(SceneImport* import, MatrixStack4x4& matrixStack, Skeleton* skeleton, int parentJointIndex, FbxSkeleton* fbxSkeleton, std::map<int, FbxNode*>& nodeToJointIndex)
    {
        Skeleton* returnSkeleton = nullptr;
        if (fbxSkeleton->IsSkeletonRoot())
        {
            //THIS IS NEW SKELETON
            returnSkeleton = new Skeleton();
            import->skeletons.push_back(returnSkeleton);
        }
        else
        {
            returnSkeleton = skeleton;
            ASSERT_OR_DIE(returnSkeleton != nullptr, "Return skeleton was null! (This should never happen lol)");
        }

        Matrix4x4 geotransform = GetGeometricTransform(fbxSkeleton->GetNode());
        matrixStack.Push(geotransform);
        Matrix4x4 modelSpace = matrixStack.GetTop();
        nodeToJointIndex[returnSkeleton->GetJointCount()] = fbxSkeleton->GetNode();
        returnSkeleton->AddJoint(fbxSkeleton->GetNode()->GetName(), parentJointIndex, modelSpace);
        matrixStack.Pop();

        return returnSkeleton;
    }

    //-----------------------------------------------------------------------------------
    static void ImportSkeletons(SceneImport* import, FbxNode* node, MatrixStack4x4& matrixStack, Skeleton* skeleton, int parentJointIndex, std::map<int, FbxNode*>& nodeToJointIndex)
    {
        if (nullptr == node)
        {
            return;
        }

        Matrix4x4 mat = GetNodeTransform(node);
        matrixStack.Push(mat);

        //Walk the attributes, looking for doot doots.
        int attributeCount = node->GetNodeAttributeCount();
        for (int attributeIndex = 0; attributeIndex < attributeCount; ++attributeIndex)
        {
            FbxNodeAttribute* attribute = node->GetNodeAttributeByIndex(attributeIndex);
            if ((attribute != nullptr) && (attribute->GetAttributeType() == FbxNodeAttribute::eSkeleton))
            {
                //So we have a skeleton
                FbxSkeleton* fbxSkele = (FbxSkeleton*)attribute;
                Skeleton* newSkeleton = ImportSkeleton(import, matrixStack, skeleton, parentJointIndex, fbxSkele, nodeToJointIndex);

                //newSkeleton will either be the same skeleton passed, or a new skeleton, or no skeleton if it was a bad node.
                //If we got something back- it's what we p[ass on to the next generation.
                if (newSkeleton != nullptr)
                {
                    skeleton = newSkeleton;
                    parentJointIndex = skeleton->GetLastAddedJointIndex();
                }
            }
        }

        //do the rest of the tree
        int childCount = node->GetChildCount();
        for (int childIndex = 0; childIndex < childCount; ++childIndex)
        {
            ImportSkeletons(import, node->GetChild(childIndex), matrixStack, skeleton, parentJointIndex, nodeToJointIndex);
        }
        matrixStack.Pop();
    }

    //-----------------------------------------------------------------------------------
    static Matrix4x4 GetNodeWorldTransformAtTime(FbxNode* node, FbxTime time, const Matrix4x4& importTransform)
    {
        if (node == nullptr) 
        {
            return Matrix4x4::IDENTITY;
        }

        FbxMatrix fbx_mat = node->EvaluateGlobalTransform(time);
        Matrix4x4 engineMatrix = ToEngineMatrix(fbx_mat);
        return engineMatrix * importTransform;
    }

    //-----------------------------------------------------------------------------------
    static void ImportMotions(SceneImport* import, FbxScene* scene, Matrix4x4& matrixStackTop, std::map<int, FbxNode*>& map, float framerate)
    {
        int animationCount = scene->GetSrcObjectCount<FbxAnimStack>();
        if (animationCount == 0)
        {
            return;
        }
        if (import->skeletons.size() == 0)
        {
            return;
        }

        //Timing information for animation in this scene. How fast is the framerate in this file?
        FbxGlobalSettings& settings = scene->GetGlobalSettings();
        FbxTime::EMode timeMode = settings.GetTimeMode();
        double sceneFramerate;
        if (timeMode == FbxTime::eCustom)
        {
            sceneFramerate = settings.GetCustomFrameRate();
        }
        else
        {
            sceneFramerate = FbxTime::GetFrameRate(timeMode);
        }

        //Only supporting one skeleton for now, update when needed.
        uint32_t skeletonCount = import->skeletons.size();
        Skeleton* skeleton= import->skeletons.at(0);
        ASSERT_OR_DIE(skeletonCount == 1, "Had multiple skeletons, we only support 1!");

        //Time between frames
        FbxTime advance;
        advance.SetSecondDouble((double)(1.0f / framerate));

        for (int animIndex = 0; animIndex < animationCount; ++animIndex)
        {
            //Import Motions
            FbxAnimStack* anim = scene->GetSrcObject<FbxAnimStack>();
            if (nullptr == anim)
            {
                continue;
            }

            FbxTime startTime = anim->LocalStart;
            FbxTime endTime = anim->LocalStop;
            FbxTime duration = endTime - startTime;

            scene->SetCurrentAnimationStack(anim);

            const char* motionName = anim->GetName();
            float timeSpan = duration.GetSecondDouble();
            AnimationMotion* motion = new AnimationMotion(motionName, timeSpan, framerate, skeleton);

            int jointCount = skeleton->GetJointCount();
            for (int jointIndex = 0; jointIndex < jointCount; ++jointIndex)
            {
                FbxNode* node = map[jointIndex];

                //Extracting world position
                //local, you would need to grab parent as well
                Matrix4x4* boneKeyframes = motion->GetJointKeyframes(jointIndex);

                FbxTime evalTime = FbxTime(0);
                for (uint32_t frameIndex = 0; frameIndex < motion->m_frameCount; ++frameIndex)
                {
                    Matrix4x4* boneKeyframe = boneKeyframes + frameIndex;

                    Matrix4x4 boneTransform = GetNodeWorldTransformAtTime(node, evalTime, matrixStackTop);
                    double seconds = evalTime.GetSecondDouble();
                    seconds = seconds;
                    *boneKeyframe = boneTransform;

                    evalTime += advance;
                }
            }
            import->motions.push_back(motion);
        }
    }

    //-----------------------------------------------------------------------------------
    static void ImportScene(SceneImport* import, FbxScene* scene, MatrixStack4x4& matrixStack)
    {
        std::map<int, FbxNode*> nodeToJointIndex;
        TriangulateScene(scene);
        FbxNode* root = scene->GetRootNode();
        ImportSkeletons(import, root, matrixStack, nullptr, -1, nodeToJointIndex);
        ImportSceneNode(import, root, matrixStack, nodeToJointIndex);
        //Top contains just our change of basis and scale matrices at this point
        ImportMotions(import, scene, matrixStack.GetTop(), nodeToJointIndex, 10);
    }

    //-----------------------------------------------------------------------------------
    static Matrix4x4 GetSceneBasis(FbxScene* scene)
    {
        FbxAxisSystem axisSystem = scene->GetGlobalSettings().GetAxisSystem();
        FbxAMatrix mat;
        axisSystem.GetMatrix(mat);
        return ToEngineMatrix(mat);
    }

    //-----------------------------------------------------------------------------------
    SceneImport* FbxLoadSceneFromFile(const char* fbxFilename, const Matrix4x4& engineBasis, bool isEngineBasisRightHanded, const Matrix4x4& transform)
    {
        FbxScene* scene = nullptr;
        FbxManager* fbxManager = FbxManager::Create();
        if (nullptr == fbxManager)
        {
            Console::instance->PrintLine("Could not create fbx manager.");
            DebuggerPrintf("Could not create fbx manager.");
            return nullptr;
        }

        FbxIOSettings* ioSettings = FbxIOSettings::Create(fbxManager, IOSROOT); //Name of object is blank, we don't care
        fbxManager->SetIOSettings(ioSettings);

        //Create an importer
        FbxImporter* importer = FbxImporter::Create(fbxManager, "");

        bool loadSuccessful = importer->Initialize(fbxFilename, -1, fbxManager->GetIOSettings());
        if (loadSuccessful)
        {
            //We have imported the FBX
            scene = FbxScene::Create(fbxManager, "");
            bool importSuccessful = importer->Import(scene);
            ASSERT_OR_DIE(importSuccessful, "Scene import failed!");
        }
        else
        {
            Console::instance->PrintLine(Stringf("Could not import scene: %s", fbxFilename));
            DebuggerPrintf("Could not import scene: %s", fbxFilename);
        }

        SceneImport* import = new SceneImport();
        MatrixStack4x4 matrixStack;

        matrixStack.Push(transform);

        //Set up our initial transforms
        Matrix4x4 sceneBasis = GetSceneBasis(scene);
        Matrix4x4::MatrixTranspose(&sceneBasis);

        if (!isEngineBasisRightHanded)
        {
            Vector3 forward = Matrix4x4::MatrixGetForward(&sceneBasis);
            Matrix4x4::MatrixSetForward(&sceneBasis, -forward); //3rd row or column
        }

        matrixStack.Push(sceneBasis);

        ImportScene(import, scene, matrixStack);

        FBX_SAFE_DESTROY(importer);
        FBX_SAFE_DESTROY(ioSettings);
        FBX_SAFE_DESTROY(scene);
        FBX_SAFE_DESTROY(fbxManager);

        return import;
    }


#else

    void FbxListScene(const char*) {};
    SceneImport* FbxLoadSceneFromFile(const char*, const Matrix4x4&, bool, const Matrix4x4&) { return nullptr; };

#endif
