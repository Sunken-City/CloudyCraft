#include "Engine/Renderer/AnimationMotion.hpp"
#include "Engine/Renderer/Skeleton.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/BinaryReader.hpp"
#include "Engine/Input/BinaryWriter.hpp"
#include "Engine/Input/Console.hpp"
#include <vector>

extern Skeleton* g_loadedSkeleton;
AnimationMotion* g_loadedMotion = nullptr;
std::vector<AnimationMotion*>* g_loadedMotions = nullptr;

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(saveMotion)
{
    if (!args.HasArgs(1))
    {
        Console::instance->PrintLine("saveMotion <filename>", RGBA::RED);
        return;
    }
    std::string filename = args.GetStringArgument(0);
    if (!g_loadedSkeleton)
    {
        Console::instance->PrintLine("Error: No skeleton has been loaded yet, use fbxLoad to bring in a mesh with a skeleton first.", RGBA::RED);
        return;
    }
    g_loadedMotion->WriteToFile(filename.c_str());
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(loadMotion)
{
    if (!args.HasArgs(1))
    {
        Console::instance->PrintLine("loadMotion <filename>", RGBA::RED);
        return;
    }
    std::string filename = args.GetStringArgument(0);
    if (g_loadedMotion)
    {
        delete g_loadedMotion;
    }
    g_loadedMotion = new AnimationMotion();
    g_loadedMotion->ReadFromFile(filename.c_str());
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(combineMotions)
{
    if (!args.HasArgs(2))
    {
        Console::instance->PrintLine("combineMotions <filename1> <filename2>", RGBA::RED);
        return;
    }
    std::string filename0 = args.GetStringArgument(0);
    std::string filename1 = args.GetStringArgument(1);
    if (g_loadedMotion)
    {
        delete g_loadedMotion;
    }
    if (g_loadedMotions)
    {
        g_loadedMotions->clear();
        delete g_loadedMotions;
    }
    g_loadedMotions = new std::vector<AnimationMotion*>();
    g_loadedMotions->push_back(new AnimationMotion());
    g_loadedMotions->push_back(new AnimationMotion());
    g_loadedMotions->at(0)->ReadFromFile(filename0.c_str());
    g_loadedMotions->at(1)->ReadFromFile(filename1.c_str());
}

//-----------------------------------------------------------------------------------
AnimationMotion::AnimationMotion(const std::string& motionName, float timeSpan, float framerate, Skeleton* skeleton)
    : m_motionName(motionName)
    , m_frameCount(static_cast<uint32_t>(ceil(framerate * timeSpan)) + 1)
    , m_jointCount(skeleton->GetJointCount())
    , m_totalLengthSeconds(timeSpan)
    , m_frameTime(1.0f/framerate)
    , m_frameRate(framerate)
    , m_playbackMode(PLAYBACK_MODE::PAUSED)
    , m_keyframes(new Matrix4x4[m_frameCount * m_jointCount])
    , m_lastTime(0.0f)
{
}

//-----------------------------------------------------------------------------------
AnimationMotion::~AnimationMotion()
{
    delete[] m_keyframes;
}

//-----------------------------------------------------------------------------------
void AnimationMotion::GetFrameIndicesWithBlend(uint32_t& outFrameIndex0, uint32_t& outFrameIndex1, float& outBlend, float inTime)
{
    uint32_t frameIndex0 = (uint32_t)floor(inTime / m_frameTime);
    uint32_t frameIndex1 = frameIndex0 + 1;

    if (frameIndex0 == (m_frameCount - 1))
    {
        frameIndex1 = m_frameCount - 1;
        outBlend = 0.0f;
    }
    else if (frameIndex0 == (m_frameCount - 2))
    {
        float lastFrameTime = m_totalLengthSeconds - (m_frameTime * frameIndex0);
        outBlend = fmodf(inTime, m_frameTime) / lastFrameTime;
        outBlend = MathUtils::Clamp(outBlend, 0.0f, 1.0f);
    }
    else
    {
        outBlend = fmodf(inTime, m_frameTime) / m_frameTime;
    }

    outFrameIndex0 = frameIndex0;
    outFrameIndex1 = frameIndex1;
}

//-----------------------------------------------------------------------------------
Matrix4x4* AnimationMotion::GetJointKeyframes(uint32_t jointIndex)
{
    return m_keyframes + (m_frameCount * jointIndex);
}

//-----------------------------------------------------------------------------------
void AnimationMotion::ApplyMotionToSkeleton(Skeleton* skeleton, float time)
{
    uint32_t frame0 = 0;
    uint32_t frame1 = 0;
    float blend;

    if (m_playbackMode == PLAYBACK_MODE::PAUSED)
    {
        time = m_lastTime;
    }
    else
    {
        m_lastTime = fmodf(time, m_totalLengthSeconds);
    }

    if (m_playbackMode == PLAYBACK_MODE::CLAMP)
    {
        time = fmodf(time, m_totalLengthSeconds);
        if (time > m_totalLengthSeconds)
        {
            time = m_totalLengthSeconds;
        }
    }
    else if (m_playbackMode == PLAYBACK_MODE::LOOP)
    {
        if (time > m_totalLengthSeconds)
        {
            time = fmodf(time, m_totalLengthSeconds);
        }
    }
    else if (m_playbackMode == PLAYBACK_MODE::PING_PONG)
    {
        if (time > m_totalLengthSeconds)
        {
            float animTime = fmodf(time, m_totalLengthSeconds * 2.0f) - m_totalLengthSeconds;
            if (animTime > 0.0f)
            {
                time = fmodf(time, m_totalLengthSeconds);
            }
            else if (animTime < 0.0f)
            {
                time = m_totalLengthSeconds - fmodf(time, m_totalLengthSeconds);
            }
        }
    } 

    GetFrameIndicesWithBlend(frame0, frame1, blend, time);

    uint32_t jointCount = skeleton->GetJointCount();
    for (uint32_t jointIndex = 0; jointIndex < jointCount; ++jointIndex)
    {
        Matrix4x4* jointKeyframes = GetJointKeyframes(jointIndex);
        Matrix4x4& matrix0 = jointKeyframes[frame0];
        Matrix4x4& matrix1 = jointKeyframes[frame1];

        Matrix4x4 newModel = Matrix4x4::MatrixLerp(matrix0, matrix1, blend);

        //Needs to set bone to model matrix
        //(Or set your matrix tree's world to this, and set
        //bone to model on Skelelton world's array
        skeleton->m_boneToModelSpace[jointIndex] = newModel; //SetJointWorldTransform(jointIndex, newModel);
    }
}

//-----------------------------------------------------------------------------------
void AnimationMotion::ApplyMotionToSkeleton(Skeleton* skeleton, float time, BoneMask& mask)
{
    uint32_t frame0 = 0;
    uint32_t frame1 = 0;
    float blend;

    if (m_playbackMode == PLAYBACK_MODE::PAUSED)
    {
        time = m_lastTime;
    }
    else
    {
        m_lastTime = fmodf(time, m_totalLengthSeconds);
    }

    if (m_playbackMode == PLAYBACK_MODE::CLAMP)
    {
        time = fmodf(time, m_totalLengthSeconds);
        if (time > m_totalLengthSeconds)
        {
            time = m_totalLengthSeconds;
        }
    }
    else if (m_playbackMode == PLAYBACK_MODE::LOOP)
    {
        if (time > m_totalLengthSeconds)
        {
            time = fmodf(time, m_totalLengthSeconds);
        }
    }
    else if (m_playbackMode == PLAYBACK_MODE::PING_PONG)
    {
        if (time > m_totalLengthSeconds)
        {
            float animTime = fmodf(time, m_totalLengthSeconds * 2.0f) - m_totalLengthSeconds;
            if (animTime > 0.0f)
            {
                time = fmodf(time, m_totalLengthSeconds);
            }
            else if (animTime < 0.0f)
            {
                time = m_totalLengthSeconds - fmodf(time, m_totalLengthSeconds);
            }
        }
    }

    GetFrameIndicesWithBlend(frame0, frame1, blend, time);

    uint32_t jointCount = skeleton->GetJointCount();
    for (uint32_t jointIndex = 0; jointIndex < jointCount; ++jointIndex)
    {
        Matrix4x4* jointKeyframes = GetJointKeyframes(jointIndex);
        Matrix4x4& matrix0 = jointKeyframes[frame0];
        Matrix4x4& matrix1 = jointKeyframes[frame1];

        Matrix4x4 newModel = Matrix4x4::MatrixLerp(matrix0, matrix1, blend);
        Matrix4x4 initialPosition = skeleton->m_boneToModelSpace[jointIndex];
        Matrix4x4 finalModel = Matrix4x4::MatrixLerp(initialPosition, newModel, mask.boneMasks[jointIndex]);

        //Needs to set bone to model matrix
        //(Or set your matrix tree's world to this, and set
        //bone to model on Skelelton world's array
        skeleton->m_boneToModelSpace[jointIndex] = finalModel; //SetJointWorldTransform(jointIndex, newModel);
    }
}

//-----------------------------------------------------------------------------------
void AnimationMotion::WriteToFile(const char* filename)
{
    BinaryFileWriter writer;
    ASSERT_OR_DIE(writer.Open(filename), "File Open failed!");
    {
        WriteToStream(writer);
    }
    writer.Close();
}

//-----------------------------------------------------------------------------------
void AnimationMotion::WriteToStream(IBinaryWriter& writer)
{
    //FILE VERSION
    //Frame Count
    //Total Length Seconds
    //Framerate
    //Frametime
    //Motion name
    //Joint count
    //Keyframes

    writer.Write<uint32_t>(FILE_VERSION);
    writer.Write<uint32_t>(m_frameCount);
    writer.Write<float>(m_totalLengthSeconds);
    writer.Write<float>(m_frameRate);
    writer.Write<float>(m_frameTime);
    writer.WriteString(m_motionName.c_str());
    writer.Write<int>(m_jointCount);
    writer.Write<PLAYBACK_MODE>(m_playbackMode);
    writer.Write<float>(m_lastTime);

    unsigned int numKeyframes = m_frameCount * m_jointCount;
    for (unsigned int index = 0; index < numKeyframes; ++index)
    {
        Matrix4x4 mat = m_keyframes[index];
        for (int i = 0; i < 16; ++i)
        {
            writer.Write<float>(mat.data[i]);
        }
    }
}

//-----------------------------------------------------------------------------------
void AnimationMotion::ReadFromStream(IBinaryReader& reader)
{
    //FILE VERSION
    //Frame Count
    //Total Length Seconds
    //Framerate
    //Frametime
    //Motion name
    //Joint count
    //Keyframes

    uint32_t fileVersion = 0;
    ASSERT_OR_DIE(reader.Read<uint32_t>(fileVersion), "Failed to read file version");
    ASSERT_OR_DIE(fileVersion == FILE_VERSION, "File version didn't match!");
    ASSERT_OR_DIE(reader.Read<uint32_t>(m_frameCount), "Failed to read frame count");
    ASSERT_OR_DIE(reader.Read<float>(m_totalLengthSeconds), "Failed to read frame count");
    ASSERT_OR_DIE(reader.Read<float>(m_frameRate), "Failed to read frame count");
    ASSERT_OR_DIE(reader.Read<float>(m_frameTime), "Failed to read frame count");
    const char* motionName = nullptr;
    reader.ReadString(motionName, 64);
    m_motionName = std::string(motionName);
    ASSERT_OR_DIE(reader.Read<int>(m_jointCount), "Failed to read frame count");
    ASSERT_OR_DIE(reader.Read<PLAYBACK_MODE>(m_playbackMode), "Failed to read playback mode");
    ASSERT_OR_DIE(reader.Read<float>(m_lastTime), "Failed to read last time");

    unsigned int numKeyframes = m_frameCount * m_jointCount; 
    m_keyframes = new Matrix4x4[numKeyframes];
    for (unsigned int index = 0; index < numKeyframes; ++index)
    {
        Matrix4x4 matrix = Matrix4x4::IDENTITY;
        for (int i = 0; i < 16; ++i)
        {
            reader.Read<float>(matrix.data[i]);
        }
        m_keyframes[index] = matrix;
    }
}

//-----------------------------------------------------------------------------------
void AnimationMotion::ReadFromFile(const char* filename)
{
    BinaryFileReader reader;
    ASSERT_OR_DIE(reader.Open(filename), "File Open failed!");
    {
        ReadFromStream(reader);
    }
    reader.Close();
}

//-----------------------------------------------------------------------------------
BoneMask::BoneMask(unsigned int numBones)
{
    //Initialize to an empty mask, everything is currently set to 0.0f (no bones are affected by motion).
    boneMasks.resize(numBones);
}

//-----------------------------------------------------------------------------------
void BoneMask::SetAllBonesTo(float boneWeight)
{
    for (float& maskWeight : boneMasks)
    {
        maskWeight = boneWeight;
    }
}
