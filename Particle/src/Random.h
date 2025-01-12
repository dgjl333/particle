	#pragma once
	#include <random>

	class Random
	{
	public:
		static void Init();
		static float GetValue();
		static float Range(float minInclusive, float maxInclusive);
		static int Range(int minInclusive, int maxExclusive);

	private:
		static std::uniform_real_distribution<float> s_dist;
		static std::mt19937 s_engine;

	};