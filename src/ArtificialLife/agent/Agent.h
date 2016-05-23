#ifndef _AGENT_H_
#define _AGENT_H_

#include <AppLib/math/MathLib.h>
#include <AppLib/math/Vector2f.h>
#include <AppLib/math/Vector3f.h>
#include <ArtificialLife/brain/Brain.h>
#include <ArtificialLife/brain/NervousSystem.h>
#include <ArtificialLife/brain/NeuronModel.h>
#include <ArtificialLife/agent/Retina.h>
#include <vector>

class Simulation;


enum class AgentCreation
{
	UNKNOWN = 0,
	BORN,
	CREATED_MATE,
	CREATED_ELITE,
	CREATED_RANDOM,
};


class Agent
{
public:
	enum { NULL_ID = 0, };

public:
	Agent(Simulation* simulation);
	~Agent();

	//-----------------------------------------------------------------------------
	// Creation.

	void Birth(AgentCreation creationType, unsigned long parent1 = NULL_ID, unsigned long parent2 = NULL_ID);
	void Grow();

	//-----------------------------------------------------------------------------
	// Update.

	void Update();
	void UpdateBrain();
	void UpdateVision(const float* pixels, int width);

	//-----------------------------------------------------------------------------
	// Events.

	void OnMate();
	void MateDelay();
	void OnEat(float foodEnergy);
	

	//-----------------------------------------------------------------------------
	// Getters.
	
	unsigned long GetID()					const { return m_id; }

	int			GetAge()					const { return m_age; }
	float		GetEnergy()					const { return m_energy; }
	Vector2f	GetPosition()				const { return m_position; }
	Vector2f	GetVelocity()				const { return m_velocity; }
	float		GetDirection()				const { return m_direction; }
	float		GetMoveSpeed()				const { return m_speed; }
	float		GetTurnSpeed()				const { return m_turnSpeed; }
	float		GetMateAmount()				const { return m_mateAmount; }
	float		GetFightAmount()			const { return m_fightAmount; }
	float		GetEatAmount()				const { return m_eatAmount; }
	
	float		GetMaxEnergy()				const { return m_maxEnergy; }
	float		GetFOV()					const { return m_retina.GetFOV(); }
	int			GetLifeSpan()				const { return m_lifeSpan; }
	float		GetSize()					const { return m_size; }
	float		GetStrength()				const { return m_strength; }
	float		GetMaxSpeed()				const { return m_maxSpeed; }
	float		GetBirthEnergyFraction()	const { return m_birthEnergyFraction; }
	
	float			GetHeuristicFitness()			const { return m_heuristicFitness; }
	AgentCreation	GetCreationType()				const { return m_creationType; }
	unsigned long	GetParentID(int parentIndex)	const { return m_parents[parentIndex]; }
	int				GetNumChildren()				const { return m_numChildren; }
	int				GetNumFoodEaten()				const { return m_numFoodEaten; }

	Retina&			GetRetina()		{ return m_retina; }
	Brain*			GetBrain()		{ return m_cns->GetBrain(); }
	BrainGenome*	GetGenome()		{ return m_brainGenome; }
	NeuronModel*	GetNeuralNet()	{ return m_cns->GetBrain()->GetNeuralNet(); }
	
	float		GetEatRadius() const;
	float		GetMateRadius() const;
	float		GetFightRadius() const;
	bool		CanMate() const;
	int			GetNumParents() const;
	
	//-----------------------------------------------------------------------------
	// Setters.

	void SetID(unsigned long id)						{ m_id = id; }
	void SetEnergy(float energy)						{ m_energy = Math::Min(energy, m_maxEnergy); }
	void AddEnergy(float amount)						{ m_energy = Math::Min(m_energy + amount, m_maxEnergy); }
	void SetPosition(const Vector2f& pos)				{ m_position = pos; }
	void SetVelocity(const Vector2f& velocity)			{ m_velocity = velocity; }
	void SetHeuristicFitness(float heuristicFitness)	{ m_heuristicFitness = heuristicFitness; }


private:
	Simulation* m_simulation;

	struct Nerves
	{
		// Inputs.
		// (RGB is handled in retina)
		Nerve* energy;
		Nerve* random;

		// Outputs.
		Nerve* moveSpeed;
		Nerve* turnSpeed;
		Nerve* eat;
		Nerve* mate;
		Nerve* fight;
	};

	Nerves			m_nerves;

	unsigned long	m_id;
	int				m_age;
	float			m_energy;
	int				m_mateTimer;
	int				m_mateDelay;
	Vector2f		m_position;
	Vector2f		m_velocity;
	float			m_direction;
	Retina			m_retina;
	NervousSystem*	m_cns;
	BrainGenome*	m_brainGenome;
	
	// Stats/info.
	float			m_heuristicFitness;
	AgentCreation	m_creationType;		// How this agent was created.
	unsigned long	m_parents[2];		// The IDs of parents (if created elite, parent ID #1 will be the elite's ID).
	int				m_numChildren;
	int				m_numFoodEaten;
	
	// Outputs.
	float		m_speed;
	float		m_turnSpeed;
	float		m_mateAmount;
	float		m_fightAmount;
	float		m_eatAmount;
	
	// Genes.
	int			m_lifeSpan;
	float		m_strength;
	float		m_size;
	float		m_maxSpeed;
	float		m_maxTurnRate;
	float		m_birthEnergyFraction;
	float		m_maxEnergy;			// Max energy is directly related to size.
};


#endif // _AGENT_H_