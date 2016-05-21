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



	//-----------------------------------------------------------------------------
	// Getters.

	int			GetAge()		const { return m_age; }
	float		GetEnergy()		const { return m_energy; }
	float		GetMaxEnergy()	const { return m_maxEnergy; }
	Vector2f	GetPosition()	const { return m_position; }
	Vector2f	GetVelocity()	const { return m_velocity; }
	float		GetDirection()	const { return m_direction; }
	bool		IsElite()		const { return m_isElite; }
	float		GetFOV()		const { return m_retina.GetFOV(); }
	
	float		GetMoveSpeed()		const { return m_speed; }
	float		GetTurnSpeed()		const { return m_turnSpeed; }
	float		GetMateAmount()		const { return m_mateAmount; }
	float		GetFightAmount()	const { return m_fightAmount; }
	float		GetEatAmount()		const { return m_eatAmount; }

	int			GetLifeSpan()	const { return m_lifeSpan; }
	float		GetHeuristicFitness()	const { return m_heuristicFitness; }

	unsigned long GetID()		const { return m_id; }
	
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

	bool CanMate() const { return (m_mateTimer <= 0); }
	void OnMate();
	void OnEat();
	
	//-----------------------------------------------------------------------------

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

	Simulation* m_simulation;

	Brain*			m_brain;
	BrainGenome*	m_brainGenome;

public:
	Vector3f	m_visionNeurons[10];

	unsigned long m_id;
};


#endif // _AGENT_H_