#include "Time.h"

std::chrono::steady_clock::time_point Time::s_startTime;
std::chrono::steady_clock::time_point Time::s_lastFrameTime;
std::chrono::duration<double> Time::s_time;
double Time::s_deltaTime = 0.01;

void Time::Init()
{
	s_startTime = std::chrono::high_resolution_clock::now();
	s_lastFrameTime = s_startTime;
}

void Time::Update()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	s_time = currentTime - s_startTime;

	s_deltaTime = std::chrono::duration<double>(currentTime - s_lastFrameTime).count();
	s_lastFrameTime = std::chrono::high_resolution_clock::now();
}

double Time::GetTime()
{
	return s_time.count();
}