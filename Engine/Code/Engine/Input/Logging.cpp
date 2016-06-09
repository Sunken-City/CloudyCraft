#include "Engine/Input/Logging.hpp"

extern bool g_isQuitting;
Logger* Logger::instance = nullptr;

//-----------------------------------------------------------------------------------
Logger::Logger()
	: m_loggingThread(LoggerThreadMain)
{

}

//-----------------------------------------------------------------------------------
Logger::~Logger()
{
	//Wait for the thread to finish shutting down, then continue.
	if (m_loggingThread.joinable())
	{
		m_loggingThread.join();
	}
}

//-----------------------------------------------------------------------------------
void LoggerThreadMain()
{
	while (!g_isQuitting)
	{
		SwitchToThread();
		EnterCriticalSection(&Logger::instance->m_logIOCriticalSection);
		LeaveCriticalSection(&Logger::instance->m_logIOCriticalSection);
	}
}
