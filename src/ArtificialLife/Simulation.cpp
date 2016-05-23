#include "Simulation.h"
#include <AppLib/graphics/Graphics.h>
#include <AppLib/math/MathLib.h>
#include <AppLib/math/Vector4f.h>
#include <ArtificialLife/brain/Brain.h>

SimulationParams Simulation::PARAMS;


Simulation::Simulation()
	: m_fittestList(NULL)
	, m_agentVisionPixels(NULL)
	, m_worldRenderer(this)
	, m_replayRecorder(this)
{
}

Simulation::~Simulation()
{
	delete m_agentVisionPixels; m_agentVisionPixels = NULL;
	
	// Delete all agents.
	for (unsigned int i = 0; i < m_agents.size(); i++)
		delete m_agents[i];
	m_agents.clear();

	delete m_fittestList; m_fittestList = NULL;
}

void Simulation::Initialize(const SimulationParams& params)
{
	Simulation::PARAMS = params;

	m_worldRenderer.LoadModels();
	
	m_worldAge			= 0;
	m_agentCounter		= 0;
	m_statistics		= SimulationStats();
	m_fittestList		= new FittestList(Simulation::PARAMS.numFittest);
	m_agentVisionPixels = new float[PARAMS.retinaResolution * 3 * PARAMS.maxAgents]; // 3 channels.

	// Setup OpenGL state.
	glDepthMask(true);
    glEnable(GL_DEPTH_CLAMP);
		
	//-----------------------------------------------------------------------------
	// Initialize world.
	
	Random::SeedTime();

	// Create initial food.
	m_food.resize(Simulation::PARAMS.initialFoodCount);
	for (int i = 0; i < Simulation::PARAMS.initialFoodCount; i++)
	{
		m_food[i].Randomize();
		m_food[i].SetPosition(Vector2f(
			Random::NextFloat() * PARAMS.worldWidth,
			Random::NextFloat() * PARAMS.worldHeight));
	}
	
	// Create initial agents with random genomes.
	for (int i = 0; i < Simulation::PARAMS.initialNumAgents; i++)
	{
		Agent* agent = new Agent(this);
		agent->GetGenome()->Randomize();
		agent->Grow();
		agent->SetPosition(Vector2f(
			Random::NextFloat() * PARAMS.worldWidth,
			Random::NextFloat() * PARAMS.worldHeight));
		m_agents.push_back(agent);
	}
}


//-----------------------------------------------------------------------------
// Update World
//-----------------------------------------------------------------------------

void Simulation::Update()
{
	m_worldAge++;

	PARAMS.worldWidth  = 1100 + Math::Min(m_worldAge / 50000.0f, 1.0f) * 1600;
	PARAMS.worldHeight = 1100 + Math::Min(m_worldAge / 50000.0f, 1.0f) * 1600;

	UpdateAgents();
	UpdateSteadyStateGA();
	UpdateFood();
}

void Simulation::UpdateAgents()
{
	// Update all agents.
	for (unsigned int i = 0; i < m_agents.size(); i++)
	{
		Agent* agent = m_agents[i];
		Vector2f agentPos = agent->GetPosition();
		
		// Find nearby food to eat.
		for (unsigned int j = 0; j < m_food.size(); j++)
		{
			float distToFood = Vector2f::Dist(agentPos, m_food[j].GetPosition());
			if (distToFood < agent->GetEatRadius() + m_food[j].GetRadius() && agent->GetEatAmount() > 0.3f)
			{
				agent->OnEat(m_food[j].Eat(0.04f) * agent->GetEatAmount());

				if (m_food[j].IsDepleted())
				{
					m_food.erase(m_food.begin() + j);
					j--;
				}
			}
		}
		
		// Update the agent.
		agent->Update();
		
		// Kill the agent if its energy drops below zero.
		if (agent->GetEnergy() <= 0.0f || agent->GetAge() >= agent->GetLifeSpan())
		{
			if (agent->GetAge() >= agent->GetLifeSpan())
				m_statistics.numAgentsDeadOldAge++;
			else if (agent->GetEnergy() <= 0.0f)
				m_statistics.numAgentsDeadEnergy++;

			Kill(agent);
			m_agents.erase(m_agents.begin() + i);
			i--;
		}
	}
	
	float mateThreshhold = 0.6f;

	// Mate agents.
	int numAgents = (int) m_agents.size();
	for (int i = 0; i < numAgents; i++)
	{
		Agent* mommy = m_agents[i];

		for (int j = 0; j < numAgents; j++)
		{
			if (i == j)
				continue;

			Agent* daddy = m_agents[j];
			float dist = Vector2f::Dist(mommy->GetPosition(), daddy->GetPosition());
				
			if (dist < mommy->GetMateRadius() + daddy->GetMateRadius() &&
				mommy->CanMate() &&
				daddy->CanMate() &&
				mommy->GetMateAmount() > mateThreshhold &&
				daddy->GetMateAmount() > mateThreshhold)// &&
				//Random::NextFloat() < (mommy->GetMateAmount() * daddy->GetMateAmount()) * 0.001f)
			{
				Agent* child = Mate(mommy, daddy);
				if (child != NULL)
				{
					child->SetPosition((mommy->GetPosition() + daddy->GetPosition()) * 0.5f);
					m_agents.push_back(child);
				}
				break;
			}
		}
	}
}

void Simulation::UpdateFood()
{
	// TODO: Better food.

	// Spawn food at a constant rate.
	if ((int) m_food.size() < Simulation::PARAMS.minFood && m_worldAge % 4 == 0)
	{
		Food food;
		food.SetPosition(Vector2f(
			Random::NextFloat() * PARAMS.worldWidth,
			Random::NextFloat() * PARAMS.worldHeight));
		m_food.push_back(food);
	}
}

void Simulation::UpdateSteadyStateGA()
{
	// Steady state GA for when the population is too small.
	if (m_fittestList->GetSize() > 1)
	{
		while ((int) m_agents.size() < Simulation::PARAMS.minAgents)
		{
			Agent* child = new  Agent(this);
			
			int numAgentsCreated = m_statistics.numAgentsCreatedElite +
								   m_statistics.numAgentsCreatedMate +
								   m_statistics.numAgentsCreatedRandom;
			
			if (Random::NextFloat() < 0.5f) // TODO: magic number
			{
				// Mate two agents.
				int iParent, jParent;
				PickParentsUsingTournament(m_fittestList->GetSize(), &iParent, &jParent);
				child->GetGenome()->Crossover(
					m_fittestList->GetByRank(iParent)->genome,
					m_fittestList->GetByRank(jParent)->genome);
				child->GetGenome()->Mutate();
				m_statistics.numAgentsCreatedMate++;
			}
			else
			{
				// Create a random agent.
				child->GetGenome()->Randomize();
				m_statistics.numAgentsCreatedRandom++;
			}
			
			// TODO: Revive a fittest agent.
			//m_numAgentsCreatedElite++;

			child->Grow();
			child->SetPosition(Vector2f(
				Random::NextFloat() * PARAMS.worldWidth,
				Random::NextFloat() * PARAMS.worldHeight));
			
			m_agents.push_back(child);
		}
	}
}

void Simulation::PickParentsUsingTournament(int numInPool, int* iParent, int* jParent)
{
	*iParent = numInPool-1;
	int tournamentSize = 5;
	for (int z = 0; z < tournamentSize; z++)
	{
		int r = Random::NextInt(0, numInPool);
		if (*iParent > r)
			*iParent = r;
	}

	do
	{
		*jParent = numInPool-1;
		for (int z = 0; z < tournamentSize; z++)
		{
			int r = Random::NextInt(0, numInPool);
			if (*jParent > r)
				*jParent = r;
		}
	}
	while (*jParent == *iParent);
}


//-----------------------------------------------------------------------------
// Agents.
//-----------------------------------------------------------------------------

Agent* Simulation::Mate(Agent* mommy, Agent* daddy)
{
	if ((int) m_agents.size() + 1 > Simulation::PARAMS.maxAgents)
	{
		m_statistics.numBirthsDenied++;
		std::cout << "Birth denied!" << std::endl;
		mommy->MateDelay();
		daddy->MateDelay();
		return NULL;
	}

	if (!mommy->CanMate() || mommy->GetEnergy() <= 0.0f ||
		!daddy->CanMate() || daddy->GetEnergy() <= 0.0f)
		return NULL;

	float mommyEnergy = mommy->GetEnergy() * mommy->GetGenome()->GetBirthEnergyFraction();
	float daddyEnergy = daddy->GetEnergy() * daddy->GetGenome()->GetBirthEnergyFraction();
	float childEnergy = mommyEnergy + daddyEnergy;
	
	// Mating costs energy.
	mommy->AddEnergy(-mommyEnergy);
	daddy->AddEnergy(-daddyEnergy);

	// Create the child.
	Agent* child = new Agent(this);
	child->GetGenome()->Crossover(
		mommy->GetGenome(),
		daddy->GetGenome());
	child->GetGenome()->Mutate();
	child->Grow();
	child->SetEnergy(childEnergy);
	child->SetPosition((mommy->GetPosition() + daddy->GetPosition()) * 0.5f);
	
	mommy->OnMate();
	daddy->OnMate();

	std::cout << "An agent was born!" << std::endl;
	
	m_statistics.numAgentsBorn++;

	return child;
}

void Simulation::Kill(Agent* agent)
{
	agent->SetHeuristicFitness(agent->GetHeuristicFitness() + (agent->GetAge() * Simulation::PARAMS.ageFitnessParam));
	agent->SetHeuristicFitness(agent->GetHeuristicFitness() + (agent->GetEnergy() * Simulation::PARAMS.energyFitnessParam));

	m_fittestList->Update(agent, agent->GetHeuristicFitness());
	
	std::cout << "Killing agent #" << agent->GetID() << " with fitness " << agent->GetHeuristicFitness() << std::endl;

	delete agent;
}


//-----------------------------------------------------------------------------
// Agent Vision.
//-----------------------------------------------------------------------------

// Render the vision of all agents.
void Simulation::RenderAgentsVision(Graphics* g)
{
	// Render each agent's vision.
	for (unsigned int i = 0; i < m_agents.size(); i++)
	{
		Agent* agent = m_agents[i];

		float fovY = 0.01f; // TODO: magic number: agent FOV-Y.

		// Setup the camera for the agent.
		Camera agentCam;
		agentCam.projection = Matrix4f::CreatePerspectiveXY(
			agent->GetFOV(), fovY, 0.1f, 1000.0f);
		agentCam.position.SetXY(agent->GetPosition());
		agentCam.position.z = 3.0f;
		agentCam.rotation = Quaternion::IDENTITY;
		agentCam.rotation.Rotate(Vector3f::UNITZ, Math::HALF_PI);
		agentCam.rotation.Rotate(Vector3f::UNITY, Math::HALF_PI);
		agentCam.rotation.Rotate(Vector3f::UNITZ, agent->GetDirection());

		// Render the world from the agent's point-of-view.
		Viewport vp(0, i, Simulation::PARAMS.retinaResolution, 1);
		g->SetViewport(vp, true, false);
		m_worldRenderer.RenderWorld(g, &agentCam, agent);
	}
			
	// Read the pixels that were rendered.
	// FIXME: Undefined behavior when window is minimized.
	Viewport vp(0, 0, Simulation::PARAMS.retinaResolution, (int) m_agents.size());
	g->SetViewport(vp, true, false);
	glReadPixels(vp.x, vp.y, vp.width, vp.height, GL_RGB, GL_FLOAT, m_agentVisionPixels);
	
	// Update the agents' visions with the pixel data.
	for (unsigned int i = 0; i < m_agents.size(); i++)
	{
		int offset = i * 3 * vp.width;
		m_agents[i]->UpdateVision(m_agentVisionPixels + offset, vp.width);
	}
}



Agent* Simulation::GetAgent(unsigned long agentID)
{
	for (auto it = agents_begin(); it != agents_end(); ++it)
	{
		Agent* agent = *it;
		if (agent->GetID() == agentID)
			return agent;
	}
	return NULL;
}

