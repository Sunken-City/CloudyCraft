#pragma once
#include <chrono>
#include <vector>

typedef unsigned int ProfilingID;

struct TimingInfo
{
public:
	TimingInfo() : m_lastSample(0.0), m_numSamples(0), m_minSample(0.0), m_maxSample(0.0), m_averageSample(0.0) {};
	void AddSample(double sampleTime);

	double m_lastSample;
	unsigned long long m_numSamples;
	double m_minSample;
	double m_maxSample;
	double m_averageSample;
	std::chrono::high_resolution_clock::time_point m_start;
	std::chrono::high_resolution_clock::time_point m_end;
};

void StartTiming();
double EndTiming();
void ResetStats();
ProfilingID RegisterProfilingChannel();
void StartTiming(ProfilingID id);
void EndTiming(ProfilingID id);

extern std::chrono::high_resolution_clock::time_point profilingStartTime;
extern std::chrono::high_resolution_clock::time_point profilingEndTime;
extern std::vector<TimingInfo> profilingResults;