#ifndef _NERVOUS_SYSTEM_H_
#define _NERVOUS_SYSTEM_H_

#include <ArtificialLife/brain/Nerve.h>
#include <vector>

class BrainGenome;
class Brain;


class NervousSystem
{
public:
	typedef std::vector<Nerve*> nerve_list;

public:
	NervousSystem();
	~NervousSystem();
	
	nerve_list::iterator nerves_begin();
	nerve_list::iterator nerves_end();

	Nerve* GetNerve(int index);
	Nerve* CreateNerve(NerveType type, int firstNeuron, int numNeurons);
	
	void Grow(BrainGenome* genome);
	void PreBirth();

	Brain* GetBrain() { return m_brain; }

private:
	nerve_list	m_nerves;
	Brain*		m_brain;
};


#endif // _NERVOUS_SYSTEM_H_