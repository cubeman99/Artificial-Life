#include "NervousSystem.h"
#include <ArtificialLife/genome/BrainGenome.h>
#include <ArtificialLife/brain/Brain.h>


NervousSystem::NervousSystem()
{
	m_brain = new Brain(this);
}

NervousSystem::~NervousSystem()
{
	delete m_brain; m_brain = NULL;

	for (unsigned int i = 0; i < m_nerves.size(); i++)
		delete m_nerves[i];
}

NervousSystem::nerve_list::iterator NervousSystem::nerves_begin()
{
	return m_nerves.begin();
}

NervousSystem::nerve_list::iterator NervousSystem::nerves_end()
{
	return m_nerves.end();
}

Nerve* NervousSystem::GetNerve(int index)
{
	return m_nerves[index];
}

Nerve* NervousSystem::CreateNerve(NerveType type, int firstNeuron, int numNeurons)
{
	Nerve* nerve = new Nerve(type, firstNeuron, numNeurons);
	m_nerves.push_back(nerve);
	return nerve;
}


void NervousSystem::Grow(BrainGenome* genome)
{
	m_brain->Grow(genome);
}

void NervousSystem::PreBirth()
{
	m_brain->PreBirth();
}
