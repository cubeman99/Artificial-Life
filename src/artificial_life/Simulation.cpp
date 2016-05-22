#include "Simulation.h"
#include "application/Graphics.h"
#include "math/MathLib.h"
#include "math/Vector4f.h"
#include "util/Timing.h" 
#include "Brain.h"
#include <algorithm> // for std::sort

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

	delete m_font; m_font = NULL;

	// Delete all agents.
	for (unsigned int i = 0; i < m_agents.size(); i++)
		delete m_agents[i];
	m_agents.clear();

	delete m_fittestList; m_fittestList = NULL;
}



void Simulation::Initialize()
{
	Random::SeedTime();
	
	m_worldRenderer.LoadModels();

	//-----------------------------------------------------------------------------
	// Reset simulation variables.

	m_worldAge					= 0;
	m_agentCounter				= 0;
	m_numAgentsBorn				= 0;
	m_numAgentsDeadOldAge		= 0;
	m_numAgentsDeadEnergy		= 0;
	m_numAgentsCreatedElite		= 0;
	m_numAgentsCreatedMate		= 0;
	m_numAgentsCreatedRandom	= 0;
	m_numBirthsDenied			= 0;

	//-----------------------------------------------------------------------------
	// Simulation globals.
	
	PARAMS.worldWidth				= 1300;
	PARAMS.worldHeight				= 1300;
	PARAMS.boundaryType				= BoundaryType::BOUNDARY_TYPE_WRAP;

	PARAMS.minAgents				= 45;//35;
	PARAMS.maxAgents				= 150;//120;
	PARAMS.initialNumAgents			= 45;

	PARAMS.minFood					= 220;
	PARAMS.maxFood					= 300;
	PARAMS.initialFoodCount			= 220;
		
	//-----------------------------------------------------------------------------
	// Energy and fitness parameters.
    
	PARAMS.numFittest				= 10;
	PARAMS.pairFrequency			= 100;
	PARAMS.eliteFrequency			= 2;

	// Parameters for measuring an agent's fitness.
	PARAMS.eatFitnessParam			= 1.0f;
	PARAMS.mateFitnessParam			= 10.0f;
	PARAMS.moveFitnessParam			= 1.0f / 800.0f;
	PARAMS.energyFitnessParam		= 2.0f;
	PARAMS.ageFitnessParam			= 0.03f;
	
	// Energy costs.
	PARAMS.energyCostEat			= 0.0f;
	PARAMS.energyCostMate			= 0.002f;
	PARAMS.energyCostFight			= 0.002f;
	PARAMS.energyCostMove			= 0.0005f;//0.002f;
	PARAMS.energyCostTurn			= 0.0005f; //0.002f;
	PARAMS.energyCostNeuron			= 0.0f; // TODO: find a value for this.
	PARAMS.energyCostSynapse		= 0.0f; // TODO: find a value for this.
	PARAMS.energyCostExist			= 0.0005f;

	//float maxsynapse2energy; // (amount if all synapses usable)
	//float maxneuron2energy;

	//-----------------------------------------------------------------------------
	// Agent configuration.
	
	PARAMS.mateWait					= 120;
	PARAMS.initialMateWait			= 120;
	PARAMS.retinaResolution			= 16;
	PARAMS.retinaVerticalFOV		= 0.01f;

	//-----------------------------------------------------------------------------
	// Agent gene ranges.

	PARAMS.minFOV					= 20.0f * Math::DEG_TO_RAD;
	PARAMS.maxFOV					= 130.0f * Math::DEG_TO_RAD;
	PARAMS.minStrength				= 0.0f;
	PARAMS.maxStrength				= 1.0f;
	PARAMS.minSize					= 0.7f;
	PARAMS.maxSize					= 1.6f;
	PARAMS.minMaxSpeed				= 1.0f;
	PARAMS.maxMaxSpeed				= 2.5f;
	PARAMS.minMutationRate			= 0.01f;
	PARAMS.maxMutationRate			= 0.1f;
	PARAMS.minNumCrossoverPoints	= 2;
	PARAMS.maxNumCrossoverPoints	= 6; // supposed to be 8
	PARAMS.minLifeSpan				= 1500;
	PARAMS.maxLifeSpan				= 2800;
	PARAMS.minBirthEnergyFraction	= 0.1f;
	PARAMS.maxBirthEnergyFraction	= 0.7f;
	PARAMS.minVisNeuronsPerGroup	= 1;
	PARAMS.maxVisNeuronsPerGroup	= 16;
	PARAMS.minInternalNeuralGroups	= 1;
	PARAMS.maxInternalNeuralGroups	= 5;

	PARAMS.minENeuronsPerGroup		= 1;
	PARAMS.maxENeuronsPerGroup		= 6;
	PARAMS.minINeuronsPerGroup		= 1;
	PARAMS.maxINeuronsPerGroup		= 6; // supposed to be 16

	PARAMS.minConnectionDensity		= 0.0f;
	PARAMS.maxConnectionDensity		= 1.0f;
	PARAMS.minTopologicalDistortion	= 0.0f;
	PARAMS.maxTopologicalDistortion	= 1.0f;
	PARAMS.minSynapseLearningRate	= 0.0f;
	PARAMS.maxSynapseLearningRate	= 0.1f;
	
	//-----------------------------------------------------------------------------
	// Brain configuration.

	PARAMS.numInputNeurGroups		= 5; // red, green, blue, energy, random
	PARAMS.numOutputNeurGroups		= 5; // speed, turn, mate, fight, eat (MISSING focus and light).
	PARAMS.numPrebirthCycles		= 10;
	PARAMS.maxBias					= 1.0f;
	PARAMS.minBiasLearningRate		= 0.0f; // unused
	PARAMS.maxBiasLearningRate		= 0.1f; // unused
	PARAMS.logisticSlope			= 1.0f;
	PARAMS.maxWeight				= 1.0f;
	PARAMS.initMaxWeight			= 0.5f;
	PARAMS.decayRate				= 0.99f;
	
	//-----------------------------------------------------------------------------
	
	//PARAMS.minAgents				= 10;
	//PARAMS.maxAgents				= 20;
	//PARAMS.initialNumAgents			= 15;
	
	//PARAMS.minAgents				= 110;
	//PARAMS.maxAgents				= 110;
	//PARAMS.initialNumAgents			= 110;

	m_worldDimensions.x = Simulation::PARAMS.worldWidth;
	m_worldDimensions.y = Simulation::PARAMS.worldHeight;
	m_fittestList = new FittestList(Simulation::PARAMS.numFittest);
	
	m_agentVisionPixels = new float[PARAMS.retinaResolution * 3 * PARAMS.maxAgents]; // 3 channels.

	//-----------------------------------------------------------------------------

	// Setup OpenGL state.
	glDepthMask(true);
    glEnable(GL_DEPTH_CLAMP);
		
	//-----------------------------------------------------------------------------
	// Initialize world.

	// Create initial food.
	m_food.resize(Simulation::PARAMS.initialFoodCount);
	for (int i = 0; i < Simulation::PARAMS.initialFoodCount; i++)
	{
		m_food[i].SetPosition(Vector2f(
			Random::NextFloat() * m_worldDimensions.x,
			Random::NextFloat() * m_worldDimensions.y));
	}
	
	// Create initial agents with random genomes.
	for (int i = 0; i < Simulation::PARAMS.initialNumAgents; i++)
	{
		Agent* agent = new Agent(this);
		agent->GetGenome()->Randomize();
		agent->Grow();
		agent->SetPosition(Vector2f(
			Random::NextFloat() * m_worldDimensions.x,
			Random::NextFloat() * m_worldDimensions.y));
		m_agents.push_back(agent);
	}
}


//-----------------------------------------------------------------------------
// Update World
//-----------------------------------------------------------------------------

void Simulation::Update()
{
	m_worldAge++;

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
		
		// Find food near the agent.
		for (unsigned int j = 0; j < m_food.size(); j++)
		{
			float distToFood = Vector2f::Dist(agentPos, m_food[j].GetPosition());
			if (distToFood < 15.0f)
			{
				m_food[j].SetPosition(Vector2f(
					Random::NextFloat() * m_worldDimensions.x,
					Random::NextFloat() * m_worldDimensions.y));
				agent->AddEnergy(m_food[j].GetEnergyValue());
				agent->OnEat();
				m_food.erase(m_food.begin() + j);
				j--;
			}
		}
		
		// Update the agent.
		agent->Update();
		
		// Kill the agent if its energy drops below zero.
		if (agent->GetEnergy() <= 0.0f || agent->GetAge() >= agent->GetLifeSpan())
		{
			if (agent->GetAge() >= agent->GetLifeSpan())
				m_numAgentsDeadOldAge++;
			else if (agent->GetEnergy() <= 0.0f)
				m_numAgentsDeadEnergy++;

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
				
			if (dist < 60 &&
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
			Random::NextFloat() * m_worldDimensions.x,
			Random::NextFloat() * m_worldDimensions.y));
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
			
			int numAgentsCreated = m_numAgentsCreatedElite + m_numAgentsCreatedMate + m_numAgentsCreatedRandom;
			
			if (Random::NextFloat() < 0.5f) // TODO: magic number
			{
				// Mate two agents.
				int iParent, jParent;
				PickParentsUsingTournament(m_fittestList->GetSize(), &iParent, &jParent);
				child->GetGenome()->Crossover(
					m_fittestList->GetByRank(iParent)->genome,
					m_fittestList->GetByRank(jParent)->genome);
				child->GetGenome()->Mutate();
				m_numAgentsCreatedMate++;
			}
			else
			{
				// Create a random agent.
				child->GetGenome()->Randomize();
				m_numAgentsCreatedRandom++;
			}
			
			// TODO: Revive a fittest agent.
			//m_numAgentsCreatedElite++;

			child->Grow();
			child->SetPosition(Vector2f(
				Random::NextFloat() * m_worldDimensions.x,
				Random::NextFloat() * m_worldDimensions.y));
			
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
		m_numBirthsDenied++;
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
	
	m_numAgentsBorn++;

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

