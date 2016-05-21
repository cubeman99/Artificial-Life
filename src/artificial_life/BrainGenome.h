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

		// Group genes.
		FIRST_GROUP_GENE,
			GENE_NUM_EXCITATORY_NEURONS = 0,
			GENE_NUM_INHIBITORY_NEURONS,
			GENE_INITIAL_BIAS,
			GENE_BIAS_LEARNING_RATE,
		NUM_GROUP_GENES,

		// Synapse genes.
		FIRST_SYNAPSE_GENE = FIRST_GROUP_GENE + NUM_GROUP_GENES,
			GENE_CONNECTION_DENSITY = 0,
			GENE_TOPOLOGICAL_DISTORTION,
			GENE_SYNAPSE_LEARNING_RATE,
		NUM_SYNAPSE_GENES,
	};

	
	enum SynapseType
	{
		SYNAPSE_EE = 0,
		SYNAPSE_EI,
		SYNAPSE_II,
		SYNAPSE_IE,
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
	int		GetNumCrossoverPoints();
	int		GetLifespan();
	float	GetBirthEnergyFraction();
	int		GetNumRedNeurons();
	int		GetNumGreenNeurons();
	int		GetNumBlueNeurons();
	int		GetNumInternalNeuralGroups();

	//-----------------------------------------------------------------------------
	// Neurogenetics.

	Gene GetSynapseGene(GeneIndex gene, int groupFrom, int groupTo, SynapseType synapseType);
	Gene GetGroupGene(GeneIndex gene, int group);

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

	void Mutate() override;

private:
};


#endif // _BRAIN_GENOME_H_