#ifndef _SIMULATION_H_
#define _SIMULATION_H_

#include <AppLib/graphics/Graphics.h>
#include <AppLib/math/Vector2f.h>
#include <AppLib/math/Vector3f.h>
#include <AppLib/math/Vector4f.h>
#include <AppLib/math/Matrix4f.h>
#include <AppLib/math/Quaternion.h>
#include <ArtificialLife/brain/NeuronModel.h>
#include <ArtificialLife/agent/Agent.h>
#include <ArtificialLife/food/Food.h>
#include <ArtificialLife/Camera.h>
#include <ArtificialLife/FittestList.h>
#include <ArtificialLife/SimulationParams.h>
#include <ArtificialLife/WorldRenderer.h>
#include <ArtificialLife/ReplayRecorder.h>
#include <vector>



struct SimulationStats
{
	int numAgentsBorn;
	int numAgentsDeadOldAge;
	int numAgentsDeadEnergy;
	int numAgentsCreatedElite;
	int numAgentsCreatedMate;
	int numAgentsCreatedRandom;
	int numBirthsDenied;

	SimulationStats()
		: numAgentsBorn(0)
		, numAgentsDeadOldAge(0)
		, numAgentsDeadEnergy(0)
		, numAgentsCreatedElite(0)
		, numAgentsCreatedMate(0)
		, numAgentsCreatedRandom(0)
		, numBirthsDenied(0)
	{}
};


class Simulation
{
public:
	typedef std::vector<Food*> food_list;
	typedef std::vector<Agent*> agent_list;

public:
	Simulation();
	~Simulation();
	
	void Initialize(const SimulationParams& params);
	void Update();
	void RenderAgentsVision(Graphics* g);
	
	Agent* GetAgent(unsigned long agentID);

	int GetNumFood()	const { return (int) m_food.size(); }
	int GetNumAgents()	const { return (int) m_agents.size(); }
	int GetWorldAge()	const { return m_worldAge; }

	const SimulationStats& GetStatistics() const { return m_statistics; }

	WorldRenderer* GetWorldRenderer() { return &m_worldRenderer; }

	food_list::iterator		food_begin()	{ return m_food.begin(); }
	food_list::iterator		food_end()		{ return m_food.end(); }
	agent_list::iterator	agents_begin()	{ return m_agents.begin(); }
	agent_list::iterator	agents_end()	{ return m_agents.end(); }


	unsigned long GetNewAgentID()
	{
		return m_agentCounter++;
	}

	float GetEnergyScale() 
	{
		if (m_agents.size() > 80)
			return 1.0f;
		return (m_agents.size() - 45.0f) / 35.0f;
	}

protected:
	void UpdateAgents();
	void UpdateFood();
	void UpdateSteadyStateGA();

	Agent* Mate(Agent* mommy, Agent* daddy);
	void Kill(Agent* agent);
	void PickParentsUsingTournament(int numInPool, int* iParent, int* jParent);
	

private:
	agent_list			m_agents;
	food_list			m_food;
	int					m_worldAge;
	
	unsigned long		m_agentCounter;
	float*				m_agentVisionPixels;
	FittestList*		m_fittestList;
	ReplayRecorder		m_replayRecorder;
	WorldRenderer		m_worldRenderer;

	SimulationStats		m_statistics;
	
public:
	static SimulationParams PARAMS;
};


#endif // _SIMULATION_H_