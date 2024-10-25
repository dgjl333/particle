#include "Time.h"

std::chrono::steady_clock::time_point Time::m_startTime;
std::chrono::steady_clock::time_point Time::m_lastFrameTime;
std::chrono::duration<double> Time::m_time;
double Time::m_deltaTime = 0.01;

void Time::Init()
{
	m_startTime = std::chrono::high_resolution_clock::now();
	m_lastFrameTime = m_startTime;
}

void Time::Update()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	m_time = currentTime - m_startTime;

	m_deltaTime = std::chrono::duration<double>(currentTime - m_lastFrameTime).count();
	m_lastFrameTime = std::chrono::high_resolution_clock::now();
}

double Time::GetTime()
{
	return m_time.count();
}