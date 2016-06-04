#include "Engine/Core/ProfilingUtils.h"
using namespace std::chrono;

std::chrono::high_resolution_clock::time_point g_profilingStartTime;
std::chrono::high_resolution_clock::time_point g_profilingEndTime;
std::vector<TimingInfo, UntrackedAllocator<TimingInfo>> g_profilingResults;

//-----------------------------------------------------------------------------------
void StartTiming()
{
    g_profilingStartTime = high_resolution_clock::now();
}

//-----------------------------------------------------------------------------------
void StartTiming(ProfilingID id)
{
    g_profilingResults[id].m_start = high_resolution_clock::now();
}

//-----------------------------------------------------------------------------------
double EndTiming()
{
    g_profilingEndTime = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(g_profilingEndTime - g_profilingStartTime);
    return time_span.count();
}

//-----------------------------------------------------------------------------------
void EndTiming(ProfilingID id)
{
    g_profilingResults[id].m_end = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(g_profilingResults[id].m_end - g_profilingResults[id].m_start);
    g_profilingResults[id].AddSample(time_span.count());
}

//-----------------------------------------------------------------------------------
void CleanUpProfilingUtils()
{
    g_profilingResults.clear();
}

//-----------------------------------------------------------------------------------
void ResetStats()
{
    for (TimingInfo& profile : g_profilingResults)
    {
        profile.m_averageSample = profile.m_lastSample;
        profile.m_maxSample = profile.m_lastSample;
        profile.m_minSample = profile.m_lastSample;
        profile.m_numSamples = 1;
    }
}

//-----------------------------------------------------------------------------------
ProfilingID RegisterProfilingChannel()
{
    g_profilingResults.push_back(TimingInfo());
    return g_profilingResults.size() - 1;
}

//-----------------------------------------------------------------------------------
void TimingInfo::AddSample(double sampleTime)
{
    m_lastSample = sampleTime;
    m_minSample = (sampleTime < m_minSample) ? sampleTime : m_minSample;
    m_maxSample = (sampleTime > m_maxSample) ? sampleTime : m_maxSample;
// 	double currentRollingAverage = m_averageSample;
// 	double currentRollingAverageExpanded = currentRollingAverage * m_numSamples;
    m_averageSample *= 0.97;
    m_averageSample += (0.03 * m_lastSample);
    m_numSamples++;
// 	m_averageSample = (currentRollingAverageExpanded + m_lastSample) / m_numSamples;
}
