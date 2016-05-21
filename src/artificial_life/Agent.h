#ifndef _AGENT_H_
#define _AGENT_H_

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
	void PreBirth();
	void Reset();
	void Update(float timeDelta);
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
	
	bool CanMate() const;	

	Retina&			GetRetina()			{ return m_retina; }
	Brain*			GetBrain()			{ return m_brain; }
	BrainGenome*	GetBrainGenome()	{ return m_brainGenome; }


	//-----------------------------------------------------------------------------
	// Setters.

	void SetID(unsigned long id)						{ m_id = id; }

	void SetEnergy(float energy)						{ m_energy = energy; }
	void AddEnergy(float amount)						{ m_energy += amount; }
	void SetPosition(const Vector2f& pos)				{ m_position = pos; }
	void SetVelocity(const Vector2f& velocity)			{ m_velocity = velocity; }
	void SetElite(bool isElite)							{ m_isElite = isElite; }
	void SetHeuristicFitness(float heuristicFitness)	{ m_heuristicFitness = heuristicFitness; }
	

private:
	//float		m_radius;

	Vector2f	m_position;
	Vector2f	m_velocity;
	float		m_direction;

	// Outputs.
	float		m_speed;
	float		m_turnSpeed;
	float		m_mateAmount;
	float		m_fightAmount;
	float		m_eatAmount;

	float		m_energy;
	float		m_maxEnergy; // Max energy is directly related to size.

	int			m_age;
	int			m_mateTimer;
	int			m_mateDelay;

	Retina		m_retina;
	
	bool		m_isElite;

	float		m_heuristicFitness;
	int			m_lifeSpan;
	float		m_strength;
	float		m_size;
	float		m_maxSpeed;
	float		m_birthEnergyFraction;

	Simulation* m_simulation;

	Brain*			m_brain;
	BrainGenome*	m_brainGenome;

public:
	Vector3f	m_visionNeurons[10];

	unsigned long m_id;
};


#endif // _AGENT_H_