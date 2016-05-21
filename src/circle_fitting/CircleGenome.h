#ifndef _CIRCLE_GENOME_H_
#define _CIRCLE_GENOME_H_

#include <ctime>
#include <random>
#include <string>

namespace circle_fitting
{

class Genome
{
public:
	Genome();
	Genome(int numBytes);

	int GetNumBytes() const;
	unsigned char GetByte(int index) const;
	void SetByte(int index, unsigned char value);

	bool GetBit(int bitIndex) const;
	void SetBit(int bitIndex, bool value);

	void Crossover(Genome* parent1, Genome* parent2, bool mutate);
	void Mutate();
	void Randomize();

	std::string GetBitString() const;

	float GetMutationRate() const { return m_mutationRate; }
	void SetMutationRate(float mutationRate) { m_mutationRate = mutationRate; }

	float GetFitness() const { return m_fitness; }
	void SetFitness(float fitness) { m_fitness = fitness; }
	
	// overload '<' used for sorting
	friend bool operator<(const Genome& lhs, const Genome& rhs)
	{
		return (lhs.m_fitness > rhs.m_fitness);
	}


private:
	int m_numBytes;
	//unsigned char* m_bytes;

	std::vector<unsigned char> m_bytes;

	float m_mutationRate;

	float m_fitness;
};

};

#endif // _CIRCLE_GENOME_H_