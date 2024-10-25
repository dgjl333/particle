	#pragma once
	#include <random>

	class Random
	{
	private:
		static std::uniform_real_distribution<float> m_dist;
		static std::mt19937 m_engine;

	public:
		static void Init();
		static float GetValue();
		static float Range(float minInclusive, float maxInclusive);
		static int Range(int minInclusive, int maxExclusive);
	};