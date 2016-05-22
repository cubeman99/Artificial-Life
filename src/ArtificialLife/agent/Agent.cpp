#include "Agent.h"
#include <ArtificialLife/Simulation.h>
#include <AppLib/math/MathLib.h>


Agent::Agent(Simulation* simulation)
	: m_velocity(Vector2f::ZERO)
	, m_position(Vector2f::ZERO)
	, m_direction(0.0f)
	, m_speed(0.0f)
	, m_energy(0)
	, m_isElite(false)
	, m_id(0)
	, m_simulation(simulation)
{
	m_brainGenome = new BrainGenome();

	m_brain = new Brain(m_brainGenome);

	m_id = m_simulation->GetNewAgentID();
}

Agent::~Agent()
{
	delete m_brain; m_brain = NULL;
	delete m_brainGenome; m_brainGenome = NULL;
}


// Grow an agent from its genome.
void Agent::Grow()
{
	// Grow the brain and some random signals to it.
	m_brain->Grow();
	m_brain->PreBirth();

	// Configure the retina.
	m_retina.SetFOV(m_brainGenome->GetFOV());
	m_retina.SetNumNeurons(0, m_brainGenome->GetNumRedNeurons());
	m_retina.SetNumNeurons(1, m_brainGenome->GetNumGreenNeurons());
	m_retina.SetNumNeurons(2, m_brainGenome->GetNumBlueNeurons());

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

	Reset();
}

void Agent::Reset()
{
	m_age				= 0;

	m_numFoodEaten		= 0;
	m_numChildren		= 0;

	m_energy			= 0.0f;
	m_heuristicFitness	= 0.0f;
	m_velocity			= Vector2f::ZERO;
	m_direction			= Random::NextFloat() * Math::TWO_PI;

	m_speed				= 0.0f;
	m_turnSpeed			= 0.0f;
	m_mateAmount		= 0.0f;
	m_fightAmount		= 0.0f;
	m_eatAmount			= 0.0f;

	m_mateTimer			= Simulation::PARAMS.initialMateWait;
	
	m_energy			= 10.0f; // Starting energy for generated agents (not born).


	m_energy			= m_maxEnergy;
}

void Agent::UpdateBrain()
{
	NeuronModel* neuralNet = m_brain->GetNeuralNet();
	
	//-----------------------------------------------------------------------------
	// Set the inputs.

	// Set vision input-neuron activations.
	int neuronIndex = 0;
	for (int channel = 0; channel < m_retina.GetNumChannels(); channel++)
	{
		for (int i = 0; i < m_retina.GetNumNeurons(channel); i++)
		{
			neuralNet->SetNeuronActivation(neuronIndex,
				m_retina.GetSightValue(channel, i));
			neuronIndex++;
		}
	}

	neuralNet->SetNeuronActivation(neuronIndex++, Math::Clamp(m_energy / m_maxEnergy, 0.0f, 1.0f)); // Energy
	neuralNet->SetNeuronActivation(neuronIndex++, Random::NextFloat()); // Random
		
	//-----------------------------------------------------------------------------
	// Update the brain's neural network.

	neuralNet->Update();
	
	//-----------------------------------------------------------------------------
	// Gather the outputs.

	float neuralNetOutputs[10];
	for (int i = 0; i < m_brain->GetNeuralNet()->GetDimensions().numOutputNeurons; i++)
	{
		neuralNetOutputs[i] = neuralNet->GetNeuronActivation(
			neuralNet->GetDimensions().GetOutputNeuronsBegin() + i);
	}

	m_speed			= neuralNetOutputs[0] * m_brainGenome->GetMaxSpeed();
	m_turnSpeed		= ((neuralNetOutputs[1] * 2.0f) - 1.0f) * m_maxTurnRate;
	m_mateAmount	= neuralNetOutputs[2];
	m_fightAmount	= neuralNetOutputs[3];
	m_eatAmount		= neuralNetOutputs[4];
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
			m_velocity.x = 0.0f;
			m_position.x = Simulation::PARAMS.worldWidth;
		}
		if (m_position.y >= Simulation::PARAMS.worldHeight)
		{
			m_velocity.y = 0.0f;
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

	float energyCost = Simulation::PARAMS.energyCostExist +
		(Simulation::PARAMS.energyCostEat     * m_eatAmount) +
		(Simulation::PARAMS.energyCostMate    * m_mateAmount) +
		(Simulation::PARAMS.energyCostFight   * m_fightAmount) +
		(Simulation::PARAMS.energyCostMove    * (m_speed / m_maxSpeed)) +
		(Simulation::PARAMS.energyCostTurn    * (Math::Abs(m_turnSpeed) / m_maxTurnRate)) +
		(Simulation::PARAMS.energyCostNeuron  * m_brain->GetNeuralNet()->GetDimensions().numNeurons) +
		(Simulation::PARAMS.energyCostSynapse * m_brain->GetNeuralNet()->GetDimensions().numSynapses);

	m_energy -= energyCost;// * m_simulation->GetEnergyScale();

	// Award fitness for moving.
	m_heuristicFitness += m_speed * Simulation::PARAMS.moveFitnessParam;
}

void Agent::UpdateVision(const float* pixels, int width)
{
	m_retina.Update(pixels, width);
}



bool Agent::CanMate() const
{
	return (m_mateTimer <= 0);
}

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

void Agent::OnEat()
{
	m_numFoodEaten++;
	m_heuristicFitness += Simulation::PARAMS.eatFitnessParam;
}

