#include "Random.h"

std::uniform_real_distribution<float> Random::m_dist(0.0, 1.0);  
std::mt19937 Random::m_engine;

void Random::Init()
{
	std::random_device seedGen;
	m_engine.seed(seedGen());
}

float Random::GetValue()
{
	return m_dist(m_engine);
}

float Random::Range(float minInclusive, float maxInclusive)
{
	std::uniform_real_distribution<float> dist(minInclusive, maxInclusive);
	return dist(m_engine);
}

int Random::Range(int minInclusive, int maxExclusive)
{
	std::uniform_int_distribution<int> dist(minInclusive, maxExclusive - 1);
	return dist(m_engine);
}