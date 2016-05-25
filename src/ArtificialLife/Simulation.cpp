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
	m_agentCounter		= 1; // Start at 1, 0 is reserved as the NULL ID.
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
	for (int i = 0; i < Simulation::PARAMS.initialFoodCount; i++)
	{
		Food* food = new Food();
		food->Randomize();
		food->SetPosition(Vector2f(
			Random::NextFloat() * PARAMS.worldWidth,
			Random::NextFloat() * PARAMS.worldHeight));
		m_food.push_back(food);
	}
	
	// Create initial agents with random genomes.
	for (int i = 0; i < Simulation::PARAMS.initialNumAgents; i++)
	{
		Agent* agent = new Agent(this);
		agent->GetGenome()->Randomize();
		agent->Birth(AgentCreation::CREATED_RANDOM);
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

	//PARAMS.worldWidth  = 1200 + Math::Min(m_worldAge / 800000.0f, 1.0f) * 2000;
	//PARAMS.worldHeight = 1200 + Math::Min(m_worldAge / 800000.0f, 1.0f) * 2000;

	UpdateAgents();
	UpdateSteadyStateGA();
	UpdateFood();
}

void Simulation::UpdateAgents()
{
	m_statistics.avgSize = 0.0f;
	m_statistics.avgStrength = 0.0f;
	m_statistics.avgFOV = 0.0f;
	m_statistics.avgMaxSpeed = 0.0f;
	m_statistics.avgGreenColor = 0.0f;
	m_statistics.avgMutationRate = 0.0f;
	m_statistics.avgNumCrossoverPoints = 0.0f;
	m_statistics.avgLifeSpan = 0.0f;
	m_statistics.avgBirthEnergyFraction = 0.0f;
	m_statistics.avgNumRedNeurons = 0.0f;
	m_statistics.avgNumGreenNeurons = 0.0f;
	m_statistics.avgNumBlueNeurons = 0.0f;
	m_statistics.avgNumInternalNeurGroups = 0.0f;
	m_statistics.avgNumNeurons = 0.0f;
	m_statistics.avgNumSynapses = 0.0f;
	m_statistics.totalEnergy = 0.0f;
	m_statistics.avgEnergyUsage = 0.0f;
	m_statistics.avgEatAmount = 0.0f;
	m_statistics.avgMateAmount = 0.0f;
	m_statistics.avgFightAmount = 0.0f;

	// Update all agents.
	for (unsigned int i = 0; i < m_agents.size(); i++)
	{
		Agent* agent = m_agents[i];
		Vector2f agentPos = agent->GetPosition();
		
		// Find nearby food to eat.
		for (unsigned int j = 0; j < m_food.size(); j++)
		{
			Food* food = m_food[j];
			float distToFood = Vector2f::Dist(agentPos, food->GetPosition());
			if (distToFood < agent->GetEatRadius() + food->GetRadius() && agent->GetEatAmount() > 0.3f)
			{
				agent->OnEat(m_food[j]->Eat(0.04f) * agent->GetEatAmount());

				if (food->IsDepleted())
				{
					delete food;
					food = NULL;
					m_food.erase(m_food.begin() + j);
					j--;
				}
			}
		}
		
		// Update the agent.
		agent->Update();
		
		m_statistics.avgSize += agent->GetSize();
		m_statistics.avgStrength += agent->GetStrength();
		m_statistics.avgFOV += agent->GetFOV();
		m_statistics.avgMaxSpeed += agent->GetGenome()->GetMaxSpeed();
		m_statistics.avgGreenColor += agent->GetGenome()->GetGreenColoration();
		m_statistics.avgMutationRate += agent->GetGenome()->GetMutationRate();
		m_statistics.avgNumCrossoverPoints += (float) agent->GetGenome()->GetNumCrossoverPoints();
		m_statistics.avgLifeSpan += (float) agent->GetLifeSpan();
		m_statistics.avgBirthEnergyFraction += agent->GetBirthEnergyFraction();
		m_statistics.avgNumRedNeurons += (float) agent->GetGenome()->GetNumRedNeurons();
		m_statistics.avgNumGreenNeurons += (float) agent->GetGenome()->GetNumGreenNeurons();
		m_statistics.avgNumBlueNeurons += (float) agent->GetGenome()->GetNumBlueNeurons();
		m_statistics.avgNumInternalNeurGroups += (float) agent->GetGenome()->GetNumInternalNeuralGroups();
		m_statistics.avgNumNeurons += (float) agent->GetNeuralNet()->GetDimensions().numNeurons;
		m_statistics.avgNumSynapses += (float) agent->GetNeuralNet()->GetDimensions().numSynapses;
		m_statistics.totalEnergy += agent->GetEnergy();
		m_statistics.avgEnergyUsage += agent->GetEnergyUsage();
		m_statistics.avgEatAmount += agent->GetEatAmount();
		m_statistics.avgMateAmount += agent->GetMateAmount();
		m_statistics.avgFightAmount += agent->GetFightAmount();

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
	
	float avgDiv = 1.0f / (float) m_agents.size();
	m_statistics.avgSize *= avgDiv;
	m_statistics.avgStrength *= avgDiv;
	m_statistics.avgFOV *= avgDiv;
	m_statistics.avgMaxSpeed *= avgDiv;
	m_statistics.avgGreenColor *= avgDiv;
	m_statistics.avgMutationRate *= avgDiv;
	m_statistics.avgNumCrossoverPoints *= avgDiv;
	m_statistics.avgLifeSpan *= avgDiv;
	m_statistics.avgBirthEnergyFraction *= avgDiv;
	m_statistics.avgNumRedNeurons *= avgDiv;
	m_statistics.avgNumGreenNeurons *= avgDiv;
	m_statistics.avgNumBlueNeurons *= avgDiv;
	m_statistics.avgNumInternalNeurGroups *= avgDiv;
	m_statistics.avgNumNeurons *= avgDiv;
	m_statistics.avgNumSynapses *= avgDiv;
	m_statistics.avgEnergyUsage *= avgDiv;
	m_statistics.avgEatAmount *= avgDiv;
	m_statistics.avgMateAmount *= avgDiv;
	m_statistics.avgFightAmount *= avgDiv;
	m_statistics.avgEnergy = m_statistics.totalEnergy * avgDiv;

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
	// TODO: Food patches.

	// Spawn food at a constant rate.
	if ((int) m_food.size() < Simulation::PARAMS.minFood && m_worldAge % 4 == 0)
	{
		Food* food = new Food();
		food->SetPosition(Vector2f(
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
				child->Birth(AgentCreation::CREATED_MATE,
					m_fittestList->GetByRank(iParent)->agentID,
					m_fittestList->GetByRank(jParent)->agentID);
			}
			else
			{
				// Create a random agent.
				child->GetGenome()->Randomize();
				m_statistics.numAgentsCreatedRandom++;
				child->Birth(AgentCreation::CREATED_RANDOM);
			}
			
			// TODO: Revive a fittest agent.
			//m_numAgentsCreatedElite++;
			//child->Birth(AgentCreation::CREATED_ELITE, ...);

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
	child->Birth(AgentCreation::BORN, mommy->GetID(), daddy->GetID());
	child->Grow();
	child->SetEnergy(childEnergy);
	child->SetPosition((mommy->GetPosition() + daddy->GetPosition()) * 0.5f);
	
	mommy->OnMate();
	daddy->OnMate();
	
	m_statistics.numAgentsBorn++;

	return child;
}

void Simulation::Kill(Agent*& agent)
{
	agent->SetHeuristicFitness(agent->GetHeuristicFitness() + (agent->GetAge() * Simulation::PARAMS.ageFitnessParam));
	agent->SetHeuristicFitness(agent->GetHeuristicFitness() + (agent->GetEnergy() * Simulation::PARAMS.energyFitnessParam));

	m_fittestList->Update(agent, agent->GetHeuristicFitness());

	delete agent;
	agent = NULL;
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

