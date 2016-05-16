#pragma once
#include <vector>
#include <string>
#include "Engine/Math/Matrix4x4.hpp"
#include "Engine/Input/BinaryReader.hpp"
#include "Engine/Input/BinaryWriter.hpp"

class MeshRenderer;

struct Joint
{
	Joint(const std::string& name, int parentIndex, const Matrix4x4& modelToBoneSpace, const Matrix4x4& boneToModelSpace);
	std::string m_name;
	int m_parentIndex;
	Matrix4x4 m_modelToBoneSpace;
	Matrix4x4 m_boneToModelSpace;
};

class Skeleton
{
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	Skeleton() : m_joints(nullptr), m_bones(nullptr) {};
	~Skeleton();

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	inline int GetLastAddedJointIndex() const { return m_names.size() - 1; };
	void AddJoint(const char* str, int parentJointIndex, Matrix4x4 initialBoneToModelMatrix);
	int FindJointIndex(const std::string& name);
	void Render() const;

	//GETTERS//////////////////////////////////////////////////////////////////////////
	uint32_t GetJointCount();
	Joint GetJoint(int index);

	//FILE IO//////////////////////////////////////////////////////////////////////////
	void WriteToFile(const char* filename);
	void WriteToStream(IBinaryWriter& writer);
	void ReadFromStream(IBinaryReader& reader);
	void ReadFromFile(const char* filename);

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	std::vector<std::string> m_names;
	std::vector<int> m_parentIndices;
	std::vector<Matrix4x4> m_modelToBoneSpace;
	std::vector<Matrix4x4> m_boneToModelSpace;
	mutable MeshRenderer* m_joints;
	mutable MeshRenderer* m_bones;

	static const unsigned int FILE_VERSION = 1;
	static const int INVALID_JOINT_INDEX = -1;
};