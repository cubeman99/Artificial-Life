#ifndef _OLD_GENOME_H_
#define _OLD_GENOME_H_

#include <ctime>
#include <random>
#include <string>
#include "util/Random.h"


class OldGenome
{
public:
	OldGenome();
	OldGenome(int numValues);

	void CopyFrom(OldGenome* genome);

	unsigned char	GetGeneValue(int index) const;
	int				GetGeneValue(int index, int rangeMin, int rangeMax) const;
	float			GetGeneValue(int index, float rangeMin, float rangeMax) const;

	float GetValue(int index);


	void SetValue(int index, float value);

	std::vector<float>& GetData() { return m_data; }
	void SetData(const std::vector<float>& data);
	
	float GetFitness() const { return m_fitness; }
	void SetFitness(float fitness) { m_fitness = fitness; }

	void Mutate(float mutationRate, float maxPerturbation);
	void Randomize();

	static void Crossover(OldGenome* parent1, OldGenome* parent2, OldGenome* child1, OldGenome* child2);
	
	void Crossover(OldGenome* g1, OldGenome* g2, bool mutate);

	// overload '<' used for sorting
	friend bool operator<(const OldGenome& lhs, const OldGenome& rhs)
	{
		return (lhs.m_fitness > rhs.m_fitness);
	}

private:
	std::vector<float> m_data;
	float m_fitness;
};


#endif // _OLD_GENOME_H_