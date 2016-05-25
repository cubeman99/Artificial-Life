#include "Agent.h"
#include <ArtificialLife/Simulation.h>
#include <AppLib/math/MathLib.h>


//-----------------------------------------------------------------------------
// Constructor & destructor.
//-----------------------------------------------------------------------------

Agent::Agent(Simulation* simulation)
	: m_velocity(Vector2f::ZERO)
	, m_position(Vector2f::ZERO)
	, m_direction(0.0f)
	, m_speed(0.0f)
	, m_energy(0)
	, m_id(0)
	, m_simulation(simulation)
{
	m_id = m_simulation->GetNewAgentID();
	
	m_creationType	= AgentCreation::UNKNOWN;
	m_parents[0]	= Agent::NULL_ID;
	m_parents[1]	= Agent::NULL_ID;

	m_brainGenome = new BrainGenome();
	m_cns = new NervousSystem();
}

Agent::~Agent()
{
	delete m_cns; m_cns = NULL;
	delete m_brainGenome; m_brainGenome = NULL;
}


//-----------------------------------------------------------------------------
// Agent Creation.
//-----------------------------------------------------------------------------
	
void Agent::Birth(AgentCreation creationType, unsigned long parent1, unsigned long parent2)
{
	m_creationType	= creationType;
	m_parents[0]	= parent1;
	m_parents[1]	= parent2;
}

// Grow an agent from its genome.
void Agent::Grow()
{
	// Grow the brain and some random signals to it.
	m_cns->Grow(m_brainGenome);
	m_cns->PreBirth();
	
	// Configure the nerves and retina.
	m_retina.SetFOV(m_brainGenome->GetFOV());
	m_retina.ConfigureChannel(0, m_cns->GetNerve(0));
	m_retina.ConfigureChannel(1, m_cns->GetNerve(1));
	m_retina.ConfigureChannel(2, m_cns->GetNerve(2));
	m_nerves.energy		= m_cns->GetNerve(3);
	m_nerves.random		= m_cns->GetNerve(4);
	m_nerves.moveSpeed	= m_cns->GetNerve(5);
	m_nerves.turnSpeed	= m_cns->GetNerve(6);
	m_nerves.eat		= m_cns->GetNerve(7);
	m_nerves.mate		= m_cns->GetNerve(8);
	m_nerves.fight		= m_cns->GetNerve(9);

	// Physiological genes.
	m_lifeSpan				= m_brainGenome->GetLifespan();
	m_size					= m_brainGenome->GetSize();
	m_strength				= m_brainGenome->GetStrength();
	m_birthEnergyFraction	= m_brainGenome->GetBirthEnergyFraction();

	// Genes modified by size.
	m_maxSpeed				= m_brainGenome->GetMaxSpeed() / m_size;
	m_maxTurnRate			= 0.2f / m_size;
	m_maxEnergy				= m_size * 13.0f;

	// Misc.
	m_mateDelay				= Simulation::PARAMS.mateWait;
	
	m_age				= 0;
	m_mateTimer			= Simulation::PARAMS.initialMateWait;
	m_energy			= m_maxEnergy; // Starting energy for generated agents (not born).
	m_heuristicFitness	= 0.0f;
	m_velocity			= Vector2f::ZERO;
	m_direction			= Random::NextFloat() * Math::TWO_PI;
	m_numFoodEaten		= 0;
	m_numChildren		= 0;
	m_energyUsage		= 0.0f;

	m_speed				= 0.0f;
	m_turnSpeed			= 0.0f;
	m_mateAmount		= 0.0f;
	m_fightAmount		= 0.0f;
	m_eatAmount			= 0.0f;
}


//-----------------------------------------------------------------------------
// Getters.
//-----------------------------------------------------------------------------

float Agent::GetEatRadius() const
{
	return (m_size * 13.0f);
}

float Agent::GetMateRadius() const
{
	return (m_size * 28.0f);
}

float Agent::GetFightRadius() const
{
	return (m_size * 15.0f);
}

bool Agent::CanMate() const
{
	return (m_mateTimer <= 0);
}

int Agent::GetNumParents() const
{
	if (m_creationType == AgentCreation::CREATED_MATE || m_creationType == AgentCreation::BORN)
		return 2;
	if (m_creationType == AgentCreation::CREATED_ELITE)
		return 1;
	return 0;
}


//-----------------------------------------------------------------------------
// Update.
//-----------------------------------------------------------------------------

void Agent::UpdateBrain()
{
	// Set the inputs.
	m_retina.UpdateNerves();
	m_nerves.energy->Set(Math::Clamp(m_energy / m_maxEnergy, 0.0f, 1.0f));
	m_nerves.random->Set(Random::NextFloat());
	
	// Update the brain's neural network.
	GetNeuralNet()->Update();
	
	// Get the outputs.
	m_speed			= m_nerves.moveSpeed->Get() * m_brainGenome->GetMaxSpeed();
	m_turnSpeed		= ((m_nerves.turnSpeed->Get() * 2.0f) - 1.0f) * m_maxTurnRate;
	m_eatAmount		= m_nerves.eat->Get();
	m_mateAmount	= m_nerves.mate->Get();
	m_fightAmount	= m_nerves.fight->Get();
}

void Agent::Update()
{
	if (m_energy > m_maxEnergy)
		m_energy = m_maxEnergy;

	UpdateBrain();
		
	//-----------------------------------------------------------------------------
	// Update movement.
	
	m_direction += m_turnSpeed;
	m_velocity.x = cosf(m_direction) * m_speed;
	m_velocity.y = -sinf(m_direction) * m_speed;

	if (Simulation::PARAMS.boundaryType == BoundaryType::BOUNDARY_TYPE_WRAP)
	{
		// Wrap around screen edges.
		if (m_position.x < 0.0f)
			m_position.x += Simulation::PARAMS.worldWidth;
		if (m_position.y < 0.0f)
			m_position.y += Simulation::PARAMS.worldHeight;
		if (m_position.x >= Simulation::PARAMS.worldWidth)
			m_position.x -= Simulation::PARAMS.worldWidth;
		if (m_position.y >= Simulation::PARAMS.worldHeight)
			m_position.y -= Simulation::PARAMS.worldHeight;
	}
	else if (Simulation::PARAMS.boundaryType == BoundaryType::BOUNDARY_TYPE_SOLID)
	{
		// Collide with screen edges.
		if (m_position.x < 0.0f)
		{
			m_velocity.x = 0.0f;
			m_position.x = 0.0f;
		}
		if (m_position.y < 0.0f)
		{
			m_velocity.y = 0.0f;
			m_position.y = 0.0f;
		}
		if (m_position.x >= Simulation::PARAMS.worldWidth)
		{
			m_velocity.x = Math::Min(m_velocity.x, 0.0f);
			m_position.x = Simulation::PARAMS.worldWidth;
		}
		if (m_position.y >= Simulation::PARAMS.worldHeight)
		{
			m_velocity.y = Math::Min(m_velocity.y, 0.0f);
			m_position.y = Simulation::PARAMS.worldHeight;
		}
	}

	m_position += m_velocity;
	
	if (m_mateTimer > 0)
		m_mateTimer--;
	
	m_age++;
	
	// Energy costs.
	//m_energy -= 0.003f * m_mateAmount;
	//m_energy -= 0.003f * m_fightAmount;
	//m_energy -= 0.003f * m_eatAmount;
	//m_energy -= 0.001f; // Energy cost for existing.

	m_energyUsage = Simulation::PARAMS.energyCostExist +
		(Simulation::PARAMS.energyCostEat     * m_eatAmount) +
		(Simulation::PARAMS.energyCostMate    * m_mateAmount) +
		(Simulation::PARAMS.energyCostFight   * m_fightAmount) +
		(Simulation::PARAMS.energyCostMove    * (m_speed / m_maxSpeed)) +
		(Simulation::PARAMS.energyCostTurn    * (Math::Abs(m_turnSpeed) / m_maxTurnRate)) +
		(Simulation::PARAMS.energyCostNeuron  * GetNeuralNet()->GetDimensions().numNeurons) +
		(Simulation::PARAMS.energyCostSynapse * GetNeuralNet()->GetDimensions().numSynapses);

	m_energy -= m_energyUsage * m_simulation->GetEnergyScale();

	// Award fitness for moving.
	m_heuristicFitness += m_speed * Simulation::PARAMS.moveFitnessParam;
}

void Agent::UpdateVision(const float* pixels, int width)
{
	m_retina.Update(pixels, width);
}


//-----------------------------------------------------------------------------
// Events.
//-----------------------------------------------------------------------------

void Agent::OnMate()
{
	m_numChildren++;
	m_mateTimer = m_mateDelay;
	m_heuristicFitness += Simulation::PARAMS.mateFitnessParam;
	//m_energy *= m_brainGenome->GetBirthEnergyFraction(); // Done in Simulation.cpp
}

void Agent::MateDelay()
{
	m_mateTimer = m_mateDelay;
}

void Agent::OnEat(float foodEnergy)
{
	m_numFoodEaten++;
	m_energy = Math::Min(m_energy + foodEnergy, m_maxEnergy);
	m_heuristicFitness += Simulation::PARAMS.eatFitnessParam * foodEnergy;
}

