#include "RandomIntSet.h"
#include <set>

RandomIntSet::RandomIntSet(int count)
{
	while (m_randoms.size() < count)
	{
		int tmp = rand();
		auto it = m_randoms.find(tmp);
		if (it==m_randoms.end())
		{
			m_randoms.insert(tmp);
		}
	}
}

RandomIntSet::RandomIntSet(int min, int max)
{
	for (int i = min; i < max; i++)
	{
		m_randoms.insert(i);
	}
}

int RandomIntSet::GetRandom(bool * geted)
{
	std::lock_guard<std::mutex> guard(m_mux);
	if (m_randoms.size()>0)
	{
		if (geted!=nullptr)
		{
			*geted = true;
		}
		auto ret= m_randoms.begin();
		int value = *ret;
		m_randoms.erase(ret);
		return value;
	}
	if(geted!=nullptr)
	{
		*geted = false;
	}
	return 0;
}

int RandomIntSet::GiveBack(int value)
{
	std::lock_guard<std::mutex> guard(m_mux);
	m_randoms.insert(value);
	return 0;
}

RandomIntSet::~RandomIntSet()
{
}
