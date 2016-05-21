#include "Genome.h"
#include <assert.h>
#include "util/Random.h"


Genome::Genome()
{
}

Genome::~Genome()
{
}

void Genome::InitSize(int size)
{
	m_data.resize(size);
}

void Genome::CopyFrom(Genome* genome)
{
	m_data = genome->m_data;
}

unsigned char Genome::GetGeneValue(int index) const
{
	return m_data[index];
}

int Genome::GetGeneValue(int index, int rangeMin, int rangeMax) const
{
	return (rangeMin + (int) (((float) m_data[index] / 255.0f) * (float) (rangeMax - rangeMin)));
}

float Genome::GetGeneValue(int index, float rangeMin, float rangeMax) const
{
	return (rangeMin + (((float) m_data[index] / 255.0f) * (rangeMax - rangeMin)));
}

void Genome::Randomize()
{
	// Randomize each bit.
    for (unsigned int byte = 0; byte < m_data.size(); byte++)
    {
        for (int bit = 0; bit < 8; bit++)
        {
            if (Random::NextBool())
                m_data[byte] |= char(1 << (7 - bit));
            else
                m_data[byte] &= char(255 ^ (1 << (7 - bit)));
		}
	}
}

void Genome::Mutate(float mutationRate)
{
	// Randomly flip bits.
    for (unsigned int byte = 0; byte < m_data.size(); byte++)
    {
        for (int bit = 0; bit < 8; bit++)
        {
			if (Random::NextFloat() < mutationRate)
                m_data[byte] ^= char(1 << (7 - bit));
		}
	}
}

void Genome::Crossover(Genome* g1, Genome* g2)
{
	assert(g1->GetDataSize() == g2->GetDataSize());

	// TODO: Variable number of crossover points (one gaurenteed in phsiological), get rid of crossover rate.

	float crossoverRate = 0.7f;

	if (Random::NextFloat() >= crossoverRate)
	{
		// Select a parent and copy their genes.
		Genome* parent = (Random::NextBool() ? g1 : g2);
		m_data.resize(g1->m_data.size());
		memcpy_s(&m_data[0], m_data.size(), parent->GetData(), parent->GetDataSize());
	}
	else
	{
		// Crossover the genes from each parent.
		
		int numCrossoverPoints = 1;
	
		// Determine a crossover point.
		int cp = Random::NextInt(0, (int) g1->m_data.size() - 1);
	
		m_data.resize(g1->m_data.size());

		Genome* parents[] = { g1, g2 };
		int parentIndex = (Random::NextBool() ? 0 : 1);

		for (int i = 0; i < cp; ++i)
			m_data[i] = parents[parentIndex]->m_data[i];

		for (int i = cp; i < (int) g1->m_data.size(); ++i)
			m_data[i] = parents[1 - parentIndex]->m_data[i];
	}
}
