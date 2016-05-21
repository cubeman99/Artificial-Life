#ifndef _AGENT_H_
#define _AGENT_H_

#include "math/MathLib.h"
#include "math/Vector2f.h"
#include "math/Vector3f.h"
#include "NeuronModel.h"
#include "Brain.h"
#include "Retina.h"
#include <vector>

class Simulation;


class Agent
{
public:
	Agent(Simulation* simulation);
	~Agent();

	//-----------------------------------------------------------------------------
	// Simulation.
	
	void Grow();
	void Reset();
	void Update();
	void UpdateBrain();
	void UpdateVision(const float* pixels, int width);
	void OnMate();
	void OnEat();


	//-----------------------------------------------------------------------------
	// Getters.
	
	unsigned long GetID()					const { return m_id; }

	int			GetAge()					const { return m_age; }
	float		GetEnergy()					const { return m_energy; }
	float		GetHeuristicFitness()		const { return m_heuristicFitness; }
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
	
	bool		IsElite()					const { return m_isElite; }
	

	Retina&			GetRetina()		{ return m_retina; }
	Brain*			GetBrain()		{ return m_brain; }
	BrainGenome*	GetGenome()		{ return m_brainGenome; }
	
	bool CanMate() const;


	//-----------------------------------------------------------------------------
	// Setters.

	void SetID(unsigned long id)						{ m_id = id; }
	void SetEnergy(float energy)						{ m_energy = Math::Min(energy, m_maxEnergy); }
	void AddEnergy(float amount)						{ m_energy = Math::Min(m_energy + amount, m_maxEnergy); }
	void SetPosition(const Vector2f& pos)				{ m_position = pos; }
	void SetVelocity(const Vector2f& velocity)			{ m_velocity = velocity; }
	void SetElite(bool isElite)							{ m_isElite = isElite; }
	void SetHeuristicFitness(float heuristicFitness)	{ m_heuristicFitness = heuristicFitness; }
	

private:
	Simulation* m_simulation;
	
	unsigned long m_id;
	bool		m_isElite;

	Vector2f	m_position;
	Vector2f	m_velocity;
	float		m_direction;
	
	// Outputs.
	float		m_speed;
	float		m_turnSpeed;
	float		m_mateAmount;
	float		m_fightAmount;
	float		m_eatAmount;

	int			m_age;
	float		m_energy;
	int			m_mateTimer;
	int			m_mateDelay;
	
	// Genes.
	float		m_heuristicFitness;
	int			m_lifeSpan;
	float		m_strength;
	float		m_size;
	float		m_maxSpeed;
	float		m_maxTurnRate;
	float		m_birthEnergyFraction;
	float		m_maxEnergy;			// Max energy is directly related to size.
	
	Retina			m_retina;
	Brain*			m_brain;
	BrainGenome*	m_brainGenome;
};


#endif // _AGENT_H_