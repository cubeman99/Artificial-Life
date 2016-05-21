#include "FittestList.h"
#include "Agent.h"
#include <assert.h>

FittestList::FittestList(int capacity)
	: m_capacity(capacity)
{
	m_fittest = new Fittest*[m_capacity];
	for (int i = 0; i < m_capacity; i++)
	{
		m_fittest[i] = new Fittest();
		m_fittest[i]->genome = new BrainGenome();
	}
}

FittestList::~FittestList()
{
	for (int i = 0; i < m_capacity; i++)
	{
		delete m_fittest[i]->genome;
		delete m_fittest[i];
	}
	delete [] m_fittest;
	m_fittest = NULL;
}


int FittestList::GetCapacity() const
{
	return m_capacity;
}

int FittestList::GetSize() const
{
	return m_size;
}

bool FittestList::IsFull() const
{
	return (m_size == m_capacity);
}

void FittestList::Clear()
{
	m_size = 0;
}

void FittestList::Update(Agent* agent, float fitness)
{
	if (!IsFull() || fitness > m_fittest[m_size - 1]->fitness)
	{
		// Determine the rank of the new agent.
		int rank = 0;
		for (int i = 0; i < m_size; i++)
		{
			if (fitness > m_fittest[i]->fitness)
			{
				rank = i;
				break;
			}
		}

		if (rank < 0)
		{
			assert(false); // This shouldn't happen.
		}

		// Increase the size if the list isn't full yet.
		// Otherwise, the lowest rank agent will be forgot.
		if (!IsFull())
		{
			m_size++;
		}
		
		// Shift other agents up (down in rank).
		Fittest* newElement = m_fittest[m_size - 1];
		for (int i = m_size - 1; i > rank; i--)
			m_fittest[i] = m_fittest[i - 1];
		m_fittest[rank] = newElement;
		
		// Add in the new agent at its appropriate rank.
		newElement->fitness = fitness;
		newElement->agentID	= agent->GetID();
		newElement->genome->CopyFrom(agent->GetBrainGenome());
	}
}

Fittest* FittestList::GetByRank(int rank)
{
	return m_fittest[rank];
}

