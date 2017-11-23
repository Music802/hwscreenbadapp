#pragma once
#include <set>
#include <mutex>
class RandomIntSet
{
public:
	RandomIntSet(int count);
	RandomIntSet(int min, int max);
	int GetRandom(bool *geted);
	int GiveBack(int value);
	~RandomIntSet();
private:
	std::mutex m_mux;
	std::set<int> m_randoms;
};

