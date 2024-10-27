#pragma once
#include <chrono>

class Time
{
private:
	static std::chrono::steady_clock::time_point s_startTime;
	static std::chrono::steady_clock::time_point s_lastFrameTime;
	static std::chrono::duration<double> s_time;
	static double s_deltaTime;

public:
	static void Init();
	static void Update();
	static double GetTime();
	static double GetDeltaTime() { return s_deltaTime; }
};