#pragma once
#include <chrono>

class Time
{
private:
	static std::chrono::steady_clock::time_point m_startTime;
	static std::chrono::steady_clock::time_point m_lastFrameTime;
	static std::chrono::duration<double> m_time;
	static double m_deltaTime;

public:
	static void Init();
	static void Update();
	static double GetTime();
	static double GetDeltaTime() { return m_deltaTime; }
};