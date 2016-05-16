#pragma once
#include "Engine/Math/Matrix4x4.hpp"
#include <string>
#include <vector>

class Skeleton;
class IBinaryReader;
class IBinaryWriter;

//-----------------------------------------------------------------------------------
struct BoneMask
{
    BoneMask(unsigned int numBones);
    void SetAllBonesTo(float boneWeight);

    std::vector<float> boneMasks;
};

//-----------------------------------------------------------------------------------
class AnimationMotion
{
public:
    //ENUMS//////////////////////////////////////////////////////////////////////////
    enum PLAYBACK_MODE
    {
        CLAMP,
        LOOP,
        PING_PONG,
        PAUSED,
        NUM_PLAYBACK_MODES
    };

    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    AnimationMotion() : m_playbackMode(PAUSED) {};
    AnimationMotion(const std::string& motionName, float timeSpan, float framerate, Skeleton* skeleton);
    ~AnimationMotion();

    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    void GetFrameIndicesWithBlend(uint32_t& outFrameIndex0, uint32_t& outFrameIndex1, float& outBlend, float inTime);
    Matrix4x4* GetJointKeyframes(uint32_t jointIndex);
    void ApplyMotionToSkeleton(Skeleton* skeleton, float time);
    void ApplyMotionToSkeleton(Skeleton* skeleton, float time, BoneMask& boneMask);
    
    //FILE IO//////////////////////////////////////////////////////////////////////////
    void WriteToFile(const char* filename);
    void WriteToStream(IBinaryWriter& writer);
    void ReadFromStream(IBinaryReader& reader);
    void ReadFromFile(const char* filename);

    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    uint32_t m_frameCount;
    float m_totalLengthSeconds;
    float m_frameRate;
    float m_frameTime;
    std::string m_motionName;
    int m_jointCount;
    //2D array of matrices, stride of sizeof(matrix4x4) * joint count
    Matrix4x4* m_keyframes;// [jointCount][frameCount];
    PLAYBACK_MODE m_playbackMode;
    float m_lastTime;

    const unsigned int FILE_VERSION = 1;
};