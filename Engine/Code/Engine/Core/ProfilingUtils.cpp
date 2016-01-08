#include "Engine/Core/ProfilingUtils.h"
using namespace std::chrono;

std::chrono::high_resolution_clock::time_point start;
std::chrono::high_resolution_clock::time_point end;
std::vector<TimingInfo> profilingResults;

void StartTiming()
{
	start = high_resolution_clock::now();
}

void StartTiming(ProfilingID id)
{
	profilingResults[id].m_start = high_resolution_clock::now();
}

double EndTiming()
{
	end = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(end - start);
	return time_span.count();
}

void EndTiming(ProfilingID id)
{
	profilingResults[id].m_end = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(profilingResults[id].m_end - profilingResults[id].m_start);
	profilingResults[id].AddSample(time_span.count());
}

ProfilingID RegisterProfilingChannel()
{
	profilingResults.push_back(TimingInfo());
	return profilingResults.size() - 1;
}

void TimingInfo::AddSample(double sampleTime)
{
	m_lastSample = sampleTime;
	m_minSample = (sampleTime < m_minSample) ? sampleTime : m_minSample;
	m_maxSample = (sampleTime > m_maxSample) ? sampleTime : m_maxSample;
	double currentRollingAverage = m_averageSample;
	double currentRollingAverageExpanded = currentRollingAverage * m_numSamples;
	m_numSamples++;
	m_averageSample = (currentRollingAverageExpanded + m_lastSample) / m_numSamples;
}
