#ifndef _SIMULATION_PARAMS_H_
#define _SIMULATION_PARAMS_H_


enum BoundaryType
{
	BOUNDARY_TYPE_SOLID = 0,	// Collide with the world boundaries.
	BOUNDARY_TYPE_WRAP,			// Wrap around the edges of the world boundaries.
	BOUNDARY_TYPE_DEATH,		// Kill agents that leave the world boundaries.
};
	

struct SimulationParams
{
	//-----------------------------------------------------------------------------
	// Simulation globals.
	
	float worldWidth;			// Size of world on the X axis.
	float worldHeight;			// Size of world on the Y axis.
	BoundaryType boundaryType;	// How world boundaries are handled.

	int   minAgents;			// Minimum number of agents, the steady-state GA will be used when population is below this amount.
	int   maxAgents;
	int   minFood;
	int   maxFood;
	int   initialFoodCount;
	int   initialNumAgents;
		
	//-----------------------------------------------------------------------------
	// Energy and fitness parameters.

	int   numFittest;
	int   pairFrequency;
	int   eliteFrequency;

	// Parameters for measuring an agent's fitness.
	float eatFitnessParam;		// Fitness gained for eating.
	float mateFitnessParam;		// Fitness gained for mating.
	float moveFitnessParam;		// Fitness per distance moved.
	float energyFitnessParam;	// Fitness per energy 
	float ageFitnessParam;		// Fitness per age in ticks.
	
	// Energy costs.
	float energyCostEat;
	float energyCostMate;
	float energyCostFight;
	float energyCostMove;
	float energyCostTurn;
	float energyCostNeuron;
	float energyCostSynapse;
	float energyCostExist;

	// TODO: Energy costs.

	//float maxsynapse2energy; // (amount if all synapses usable)
	//float maxneuron2energy;

	//-----------------------------------------------------------------------------
	// Agent configuration.

	int   mateWait;				// Time to wait after mating before mating again.
	int   initialMateWait;		// Time to wait before mating after birth (i.e. age of firtility).
	int   retinaResolution;		// The resolution width at which an agent's vision is renderered.
	float retinaVerticalFOV;	// Vertical field of view in radians, should be very small (like 0.01f).

	//-----------------------------------------------------------------------------
	// Agent gene ranges.

	float minFOV;
	float maxFOV;
	float minStrength;
	float maxStrength;
	float minSize;
	float maxSize;
	float minMaxSpeed;
	float maxMaxSpeed;
	float minMutationRate;
	float maxMutationRate;
	int   minNumCrossoverPoints;
	int   maxNumCrossoverPoints;
	int   minLifeSpan;
	int   maxLifeSpan;
	float minBirthEnergyFraction;
	float maxBirthEnergyFraction;
	int   minVisNeuronsPerGroup;
	int   maxVisNeuronsPerGroup;
	int   minInternalNeuralGroups;
	int   maxInternalNeuralGroups;

	int   minENeuronsPerGroup;
	int   maxENeuronsPerGroup;
	int   minINeuronsPerGroup;
	int   maxINeuronsPerGroup;

	float minConnectionDensity;
	float maxConnectionDensity;
	float minTopologicalDistortion;
	float maxTopologicalDistortion;
	float minSynapseLearningRate;
	float maxSynapseLearningRate;
	
	//-----------------------------------------------------------------------------
	// Brain configuration.

	int   numInputNeurGroups;
	int   numOutputNeurGroups;

	int   numPrebirthCycles;
		
	float maxBias;
	float minBiasLearningRate;
	float maxBiasLearningRate;
	//bool  outputSynapseLearning;
	//bool  synapseFromOutputNeurons;

	float logisticSlope;
	float maxWeight;
	float initMaxWeight;
	float decayRate;
};


#endif // _SIMULATION_PARAMS_H_