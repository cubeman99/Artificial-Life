#ifndef _BRAIN_GENOME_H_
#define _BRAIN_GENOME_H_

#include "Genome.h"
#include "NeuronType.h"


class BrainGenome : public Genome
{
public:
	enum GeneIndex
	{
		// Single value genes.
		GENE_SIZE = 0,
		GENE_STRENGTH,
		GENE_FOV,
		GENE_GENE_MAX_SPEED,
		GENE_GREEN_COLOR,
		GENE_MUTATION_RATE,
		GENE_NUM_CROSSOVER_POINTS,
		GENE_LIFE_SPAN,
		GENE_BIRTH_ENERGY_FRACTION,
		GENE_NUM_RED_NEURONS,
		GENE_NUM_GREEN_NEURONS,
		GENE_NUM_BLUE_NEURONS,
		GENE_NUM_INTERNAL_NEURAL_GROUPS,
		NUM_PHYSIOLOGICAL_GENES,

		// Group genes.
		GENE_NUM_EXCITATORY_NEURONS = 0,
		GENE_NUM_INHIBITORY_NEURONS,
		GENE_INITIAL_BIAS,
		GENE_BIAS_LEARNING_RATE,
		NUM_GROUP_GENES,

		// Synapse genes.
		GENE_CONNECTION_DENSITY = 0,
		GENE_TOPOLOGICAL_DISTORTION,
		GENE_SYNAPSE_LEARNING_RATE,
		NUM_SYNAPSE_GENES,
	};

	enum SynapseType
	{
		SYNAPSE_EE = 0,	// Excitatory to excitatory.
		SYNAPSE_EI,		// Excitatory to inhibitory.
		SYNAPSE_II,		// Inhibitory to inhibitory.
		SYNAPSE_IE,		// Inhibitory to excitatory.
		NUM_SYNAPSE_TYPES
	};

	enum NeurGroupType
	{
		NEURAL_GROUP_INPUT = 0,
		NEURAL_GROUP_OUTPUT,
		NEURAL_GROUP_INTERNAL,
	};

	struct NeurGroupInfo
	{
		int numNeurons;
		int numExcitatoryNeurons;
		int numInhibitoryNeurons;
		float initialBias;
		float biasLearningRate;
	};

	struct NeurGroupSynapseInfo
	{
		float connectionDensity;
		float topologicalDistortion;
		float synapseLearningRate;
	};

public:
	BrainGenome();
	~BrainGenome();

	//-----------------------------------------------------------------------------
	// Gene access.

	float	GetFOV();
	float	GetSize();
	float	GetStrength();
	float	GetMaxSpeed();
	float	GetGreenColoration();
	float	GetMutationRate();
	int		GetLifespan();
	float	GetBirthEnergyFraction();
	int		GetNumRedNeurons();
	int		GetNumGreenNeurons();
	int		GetNumBlueNeurons();
	int		GetNumInternalNeuralGroups();

	//-----------------------------------------------------------------------------
	// Neurogenetics.

	Gene GetGroupGene(GeneIndex gene, int group);
	Gene GetSynapseGene(GeneIndex gene, int groupFrom, int groupTo, SynapseType synapseType);

	NeurGroupInfo GetGroupInfo(int group);
	NeurGroupSynapseInfo GetSynapseInfo(int groupFrom, int groupTo, SynapseType synapseType);

	bool IsOutputGroup(int group);
	int GetNeuronCount(NeuronType neuronType, int group);
	int GetSynapseCount(int groupFrom, int groupTo);
	int GetSynapseCount(int groupFrom, int groupTo, SynapseType synapseType);

	static NeuronType GetSynapseNeuronType_From(SynapseType synpaseType);
	static NeuronType GetSynapseNeuronType_To(SynapseType synpaseType);

	//-----------------------------------------------------------------------------
	// Overridden methods.
	
	int GetNumCrossoverPoints() override;
	void GetCrossoverPoints(int* crossoverPoints, int numCrossoverPoints) override;
	void Mutate() override;

private:
};


#endif // _BRAIN_GENOME_H_