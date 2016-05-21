#include "OldGenome.h"
#include <string.h>


OldGenome::OldGenome()
{
}

OldGenome::OldGenome(int numValues)
{
	m_data.resize(numValues);
}

void OldGenome::CopyFrom(OldGenome* genome)
{
	m_fitness = genome->m_fitness;
	m_data = genome->m_data;
}

float OldGenome::GetValue(int index)
{
	return m_data[index];
}

unsigned char OldGenome::GetGeneValue(int index) const
{
	return 0; // TODO: migrate from flotas to bytes.
}

int OldGenome::GetGeneValue(int index, int rangeMin, int rangeMax) const
{
	return (rangeMin + (int) (m_data[index] * (float) (rangeMax - rangeMin)));
}

float OldGenome::GetGeneValue(int index, float rangeMin, float rangeMax) const
{
	return (rangeMin + (m_data[index] * (rangeMax - rangeMin)));
}

void OldGenome::SetValue(int index, float value)
{
	m_data[index] = value;
}

void OldGenome::SetData(const std::vector<float>& data)
{
	m_data = data;
}

void OldGenome::Mutate(float mutationRate, float maxPerturbation)
{
	for (unsigned int i = 0; i < m_data.size(); i++)
	{
		if (Random::NextFloat() < mutationRate)
			m_data[i] += Random::NextFloatClamped() * maxPerturbation;
	}
}

void OldGenome::Randomize()
{
	for (unsigned int i = 0; i < m_data.size(); i++)
	{
		m_data[i] += Random::NextFloatClamped();
	}
}

void OldGenome::Crossover(OldGenome* parent1, OldGenome* parent2, OldGenome* child1, OldGenome* child2)
{
	//determine a crossover point
	int cp = Random::NextInt(0, (int) parent1->m_data.size() - 1);
	
	child1->m_data.resize(parent1->m_data.size());
	child2->m_data.resize(parent1->m_data.size());

	//create the offspring
	for (int i = 0; i < cp; ++i)
	{
		child1->m_data[i] = parent1->m_data[i];
		child2->m_data[i] = parent2->m_data[i];
	}

	for (int i = cp; i < (int) parent1->m_data.size(); ++i)
	{
		child1->m_data[i] = parent2->m_data[i];
		child2->m_data[i] = parent1->m_data[i];
	}
	return;


	int numCrossoverPoints = 1;
	int* crossoverPoints = new int[numCrossoverPoints];

	// Create random crossover points.
	for (int i = 0; i < numCrossoverPoints; i++)
	{
		int point = Random::NextInt(0, parent1->m_data.size());
			
		//bool equal;
		/*do
		{
			point = rng.NextInt(0, m_numBytes);
			equal = false;

			for (int j = 0; j < i; j++)
			{
				if (point == crossoverPoints[j])
				{
					equal = true;
					break;
				}
			}
		}
		while (equal);*/

		// Insert crossover point in sorted order.
		if (i == 0)
			crossoverPoints[0] = point;
		else if (i > 0 && point >= crossoverPoints[i - 1])
			crossoverPoints[i] = point;
		else
		{
			for (int j = 0; j < i; j++)
			{
				if (point < crossoverPoints[j])
				{
					for (int k = i; k > j; k--)
						crossoverPoints[k] = crossoverPoints[k - 1];
					crossoverPoints[j] = point;
					break;
				}
				else if (j == i - 1)
					crossoverPoints[j] = point;
			}
		}
	}

	// Crossover the bits of the two parent genomes.
	OldGenome* parents[2] = { parent1, parent2 };
	int parentIndex = 0;
	
	child1->m_data.resize(parent1->m_data.size());
	child2->m_data.resize(parent1->m_data.size());

	for (int i = 0; i < numCrossoverPoints + 1; i++)
	{
		// Copy the sequence of bytes.
		int begIndex = 0;
		int endIndex = (int) parent1->m_data.size();
		if (i > 0)
			begIndex = crossoverPoints[i - 1];
		if (i < numCrossoverPoints)
			endIndex = crossoverPoints[i];

		for (int j = begIndex; j < endIndex; j++)
		{
			child1->m_data[j] = parents[parentIndex]->m_data[j];
			child2->m_data[j] = parents[1 - parentIndex]->m_data[j];
		}

		/*
		memcpy_s(&child1->m_data[0] + begIndex, child1->m_data.size(),
				 &parents[parentIndex]->m_data[0] + begIndex,
				 endIndex - begIndex);
		
		memcpy_s(&child2->m_data[0] + begIndex, child2->m_data.size(),
				 &parents[1 - parentIndex]->m_data[0] + begIndex,
				 endIndex - begIndex);
		*/

		// Switch parents for the next section.
		parentIndex = 1 - parentIndex;
	}
	
	delete [] crossoverPoints;
}

void OldGenome::Crossover(OldGenome* g1, OldGenome* g2, bool mutate)
{
	// Determine a crossover point.
	int cp = Random::NextInt(0, (int) g1->m_data.size() - 1);
	
	m_data.resize(g1->m_data.size());

	OldGenome* parents[] = { g1, g2 };
	int parentIndex = (Random::NextBool() ? 0 : 1);

	for (int i = 0; i < cp; ++i)
		m_data[i] = parents[parentIndex]->m_data[i];

	for (int i = cp; i < (int) g1->m_data.size(); ++i)
		m_data[i] = parents[1 - parentIndex]->m_data[i];
}