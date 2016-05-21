#include "Agent.h"
#include "Simulation.h"
#include "math/MathLib.h"


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
	m_brainGenome->Randomize();
	m_brain = new Brain(m_brainGenome);
	m_brain->PreBirth();

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
	// Grow the brain.
	m_brain->Grow();

	// Configure the retina.
	m_retina.SetFOV(m_brainGenome->GetFOV());
	m_retina.SetNumNeurons(0, m_brainGenome->GetNumRedNeurons());
	m_retina.SetNumNeurons(1, m_brainGenome->GetNumGreenNeurons());
	m_retina.SetNumNeurons(2, m_brainGenome->GetNumBlueNeurons());

	// Other genes.
	m_lifeSpan				= m_brainGenome->GetLifespan();
	m_size					= m_brainGenome->GetSize();
	m_strength				= m_brainGenome->GetStrength();
	m_maxSpeed				= m_brainGenome->GetMaxSpeed();
	m_birthEnergyFraction	= m_brainGenome->GetBirthEnergyFraction();
	
	m_maxEnergy = 20.0f; // TODO: max energy determined by size.
	
	m_maxEnergy = m_size * 12.0f;

	m_mateDelay = Simulation::PARAMS.mateWait;

	Reset();
}

void Agent::PreBirth()
{
	// Send some random signals to the brain.
	m_brain->PreBirth();
}


void Agent::Reset()
{
	m_age				= 0;

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
}

void Agent::Update(float timeDelta)
{
	if (m_energy > m_maxEnergy)
		m_energy = m_maxEnergy;

	//-----------------------------------------------------------------------------
	// Update brain.

	NeuronModel* neuralNet = m_brain->GetNeuralNet();
	
	// Set input activations.
	for (int i = neuralNet->GetDimensions().GetInputNeuronsBegin(); i < neuralNet->GetDimensions().GetInputNeuronsEnd(); i++)
	{
		neuralNet->SetNeuronActivation(i, Random::NextFloat());
	}
	
	// NOTE: this may need to be done twice per time step??
	//for (int k = 0; k < 2; k++)
	{
		// Set vision neuron activations.
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

		neuralNet->Update();
	}
	
	// Gather the outputs.
	std::vector<float> neuralNetOutputs;
	for (int i = 0; i < m_brain->GetNeuralNet()->GetDimensions().numOutputNeurons; i++)
	{
		neuralNetOutputs.push_back(neuralNet->GetNeuronActivation(
			neuralNet->GetDimensions().GetOutputNeuronsBegin() + i));
	}

	float maxTurnRate	= 0.2f;
	m_speed			= neuralNetOutputs[0] * m_brainGenome->GetMaxSpeed() / m_brainGenome->GetSize();
	m_turnSpeed		= ((neuralNetOutputs[1] * 2.0f) - 1.0f) * maxTurnRate / m_brainGenome->GetSize();
	m_mateAmount	= neuralNetOutputs[2];
	m_fightAmount	= neuralNetOutputs[3];
	m_eatAmount		= neuralNetOutputs[4];
	
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
	
	m_energy -= 0.003f * m_mateAmount;

	m_energy -= 0.001f;

	//m_energy -= 0.005f; // Energy cost for existing.

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
	m_mateTimer = m_mateDelay;
	m_heuristicFitness += Simulation::PARAMS.mateFitnessParam;
	//m_energy *= m_brainGenome->GetBirthEnergyFraction(); // Done in Simulation.cpp
}

void Agent::OnEat()
{
	m_heuristicFitness += Simulation::PARAMS.eatFitnessParam;
}

