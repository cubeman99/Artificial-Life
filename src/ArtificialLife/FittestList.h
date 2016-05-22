#ifndef _FITTEST_LIST_H_
#define _FITTEST_LIST_H_

#include <ArtificialLife/genome/BrainGenome.h>
#include <vector>

class Agent;


struct Fittest
{
	int		agentID;
	float	fitness;
	BrainGenome* genome;

	Fittest()
		: fitness(0.0f)
		, agentID(0)
		, genome(NULL)
	{}
};


class FittestList
{
public:
	FittestList(int capacity);
	~FittestList();

	int GetCapacity() const;
	int GetSize() const;
	bool IsFull() const;
	void Clear();

	void Update(Agent* agent, float fitness);

	Fittest* GetByRank(int rank);

private:
	int m_capacity;
	int m_size;
	Fittest** m_fittest;
};


#endif // _FITTEST_LIST_H_