#include "BrainGenome.h"
#include "Brain.h"
#include <assert.h>
#include "math/MathLib.h"
#include "Simulation.h"


BrainGenome::BrainGenome()
{
	int numGroups = Simulation::PARAMS.numInputNeurGroups +
					Simulation::PARAMS.numOutputNeurGroups +
					Simulation::PARAMS.maxInternalNeuralGroups;

	int genomeSize = FIRST_GROUP_GENE +
		(numGroups * NUM_GROUP_GENES) +
		(numGroups * numGroups * NUM_SYNAPSE_TYPES * NUM_SYNAPSE_GENES);

	InitSize(genomeSize);
}

BrainGenome::~BrainGenome()
{
}


Gene BrainGenome::GetSynapseGene(GeneIndex gene, int groupFrom, int groupTo, SynapseType synapseType)
{
	int numGroups = Simulation::PARAMS.numInputNeurGroups +
					Simulation::PARAMS.numOutputNeurGroups +
					Simulation::PARAMS.maxInternalNeuralGroups;

	int offset = FIRST_SYNAPSE_GENE + (numGroups * NUM_GROUP_GENES) +
		((groupFrom * numGroups  * NUM_SYNAPSE_TYPES) +
		(groupTo * NUM_SYNAPSE_TYPES) + (synapseType))  * NUM_SYNAPSE_GENES;

	return GetGene(offset + gene);
}

Gene BrainGenome::GetGroupGene(GeneIndex gene, int group)
{
	int offset = FIRST_GROUP_GENE + (group * NUM_GROUP_GENES) + gene;
	return GetGene(offset);
}


BrainGenome::NeurGroupInfo BrainGenome::GetGroupInfo(int group)
{
	NeurGroupInfo groupInfo;
	groupInfo.numExcitatoryNeurons = GetNeuronCount(NEURON_TYPE_EXCITATORY, group);
	groupInfo.numInhibitoryNeurons = GetNeuronCount(NEURON_TYPE_INHIBITORY, group);
	groupInfo.initialBias = GetGroupGene(GENE_INITIAL_BIAS, group).AsFloat(
		-Simulation::PARAMS.maxBias,
		Simulation::PARAMS.maxBias);
	groupInfo.biasLearningRate = GetGroupGene(GENE_BIAS_LEARNING_RATE, group).AsFloat(
		Simulation::PARAMS.minBiasLearningRate,
		Simulation::PARAMS.maxBiasLearningRate);
	return groupInfo;
}

BrainGenome::NeurGroupSynapseInfo BrainGenome::GetSynapseInfo(int groupFrom, int groupTo, SynapseType synapseType)
{
	NeurGroupSynapseInfo synapseInfo;
	synapseInfo.connectionDensity = GetSynapseGene(
		GENE_CONNECTION_DENSITY, groupFrom, groupTo, synapseType).AsFloat(
		Simulation::PARAMS.minConnectionDensity,
		Simulation::PARAMS.maxConnectionDensity);
	synapseInfo.topologicalDistortion = GetSynapseGene(
		GENE_TOPOLOGICAL_DISTORTION, groupFrom, groupTo, synapseType).AsFloat(
		Simulation::PARAMS.minTopologicalDistortion,
		Simulation::PARAMS.maxTopologicalDistortion);
	synapseInfo.synapseLearningRate = GetSynapseGene(
		GENE_SYNAPSE_LEARNING_RATE, groupFrom, groupTo, synapseType).AsFloat(
		Simulation::PARAMS.minSynapseLearningRate,
		Simulation::PARAMS.maxSynapseLearningRate);

	// Negate learning rate for inhibitory neurons.
	if (GetSynapseNeuronType_From(synapseType) == NEURON_TYPE_INHIBITORY)
	{
		synapseInfo.synapseLearningRate = Math::Min(-1.e-10f, -synapseInfo.synapseLearningRate);
	}

	return synapseInfo;
}

bool BrainGenome::IsOutputGroup(int group)
{
	return (group >= Simulation::PARAMS.numInputNeurGroups &&
			group < Simulation::PARAMS.numInputNeurGroups + Simulation::PARAMS.numOutputNeurGroups);
}


int BrainGenome::GetNeuronCount(NeuronType neuronType, int group)
{
	if (group < Simulation::PARAMS.numInputNeurGroups + Simulation::PARAMS.numOutputNeurGroups)
	{
		if (group == 0)
			return GetNumRedNeurons();
		else if (group == 1)
			return GetNumGreenNeurons();
		else if (group == 2)
			return GetNumBlueNeurons();
		else
			return 1;
	}

	if (neuronType == NEURON_TYPE_EXCITATORY)
	{
		return GetGroupGene(GENE_NUM_EXCITATORY_NEURONS, group).AsInt(
			Simulation::PARAMS.minENeuronsPerGroup,
			Simulation::PARAMS.maxENeuronsPerGroup);
	}
	else
	{
		return GetGroupGene(GENE_NUM_INHIBITORY_NEURONS, group).AsInt(
			Simulation::PARAMS.minINeuronsPerGroup,
			Simulation::PARAMS.maxINeuronsPerGroup);
	}
}


int BrainGenome::GetSynapseCount(int groupFrom, int groupTo)
{
	int count = 0;
	for (int i = 0; i < NUM_SYNAPSE_TYPES; i++)
		count += GetSynapseCount(groupFrom, groupTo, (SynapseType) i);
	return count;
}


int BrainGenome::GetSynapseCount(int groupFrom, int groupTo, SynapseType synapseType)
{
	float connectionDensity = GetSynapseGene(GENE_CONNECTION_DENSITY, groupFrom, groupTo, synapseType)
		.AsFloat(Simulation::PARAMS.minConnectionDensity, Simulation::PARAMS.maxConnectionDensity);
	
	NeuronType neuronType_from = GetSynapseNeuronType_From(synapseType);
	NeuronType neuronType_to   = GetSynapseNeuronType_To(synapseType);
	
	bool to_output = IsOutputGroup(groupTo);

	
	if (neuronType_to == NEURON_TYPE_INHIBITORY && to_output)
	{
		// As targets, the output neurons are treated exclusively as excitatory
		return 0;
	}
	
	int nFrom = GetNeuronCount(neuronType_from, groupFrom);
	int nTo   = GetNeuronCount(neuronType_to, groupTo);
	
	if (groupFrom == groupTo)
	{
		if (synapseType == SYNAPSE_IE && to_output)
		{
			// If the source and target groups are the same, and both are an output
			// group, then this will evaluate to zero.
			nTo--;
		}
		else if (neuronType_from == neuronType_to)
		{
			nTo--;
		}
	}

	return Math::NInt(nFrom * nTo * connectionDensity);
}

void BrainGenome::Mutate()
{
	Genome::Mutate(GetMutationRate());
}


NeuronType BrainGenome::GetSynapseNeuronType_From(SynapseType synpaseType)
{
	switch (synpaseType)
	{
	case SYNAPSE_EE:
	case SYNAPSE_EI:
		return NEURON_TYPE_EXCITATORY;
	case SYNAPSE_II:
	case SYNAPSE_IE:
		return NEURON_TYPE_INHIBITORY;
	default:
		assert(false);
		return NEURON_TYPE_EXCITATORY;
	}
}

NeuronType BrainGenome::GetSynapseNeuronType_To(SynapseType synpaseType)
{
	switch (synpaseType)
	{
	case SYNAPSE_EE:
	case SYNAPSE_IE:
		return NEURON_TYPE_EXCITATORY;
	case SYNAPSE_II:
	case SYNAPSE_EI:
		return NEURON_TYPE_INHIBITORY;
	default:
		assert(false);
		return NEURON_TYPE_EXCITATORY;
	}
}



float BrainGenome::GetFOV()
{
	return GetGene(GENE_SIZE).AsFloat(
		Simulation::PARAMS.minFOV,
		Simulation::PARAMS.maxFOV);
}

float BrainGenome::GetSize()
{
	return GetGene(GENE_SIZE).AsFloat(
		Simulation::PARAMS.minSize,
		Simulation::PARAMS.maxSize);
}

float BrainGenome::GetStrength()
{
	return GetGene(GENE_SIZE).AsFloat(
		Simulation::PARAMS.minStrength,
		Simulation::PARAMS.maxStrength);
}

float BrainGenome::GetMaxSpeed()
{
	return GetGene(GENE_GENE_MAX_SPEED).AsFloat(
		Simulation::PARAMS.minMaxSpeed,
		Simulation::PARAMS.maxMaxSpeed);
}

float BrainGenome::GetGreenColoration()
{
	return GetGene(GENE_GREEN_COLOR).AsFloat(
		0.0f,
		1.0f);
}

float BrainGenome::GetMutationRate()
{
	return GetGene(GENE_MUTATION_RATE).AsFloat(
		Simulation::PARAMS.minMutationRate,
		Simulation::PARAMS.maxMutationRate);
}

int BrainGenome::GetNumCrossoverPoints()
{
	return GetGene(GENE_NUM_CROSSOVER_POINTS).AsInt(
		Simulation::PARAMS.minNumCrossoverPoints,
		Simulation::PARAMS.maxNumCrossoverPoints);
}

int BrainGenome::GetLifespan()
{
	return GetGene(GENE_LIFE_SPAN).AsInt(
		Simulation::PARAMS.minLifeSpan,
		Simulation::PARAMS.maxLifeSpan);
}

float BrainGenome::GetBirthEnergyFraction()
{
	return GetGene(GENE_BIRTH_ENERGY_FRACTION).AsFloat(
		Simulation::PARAMS.minBirthEnergyFraction,
		Simulation::PARAMS.maxBirthEnergyFraction);
}

int BrainGenome::GetNumRedNeurons()
{
	return GetGene(BrainGenome::GENE_NUM_RED_NEURONS).AsInt(
		Simulation::PARAMS.minVisNeuronsPerGroup,
		Simulation::PARAMS.maxVisNeuronsPerGroup);
}

int BrainGenome::GetNumGreenNeurons()
{
	return GetGene(BrainGenome::GENE_NUM_GREEN_NEURONS).AsInt(
		Simulation::PARAMS.minVisNeuronsPerGroup,
		Simulation::PARAMS.maxVisNeuronsPerGroup);
}

int BrainGenome::GetNumBlueNeurons()
{
	return GetGene(BrainGenome::GENE_NUM_BLUE_NEURONS).AsInt(
		Simulation::PARAMS.minVisNeuronsPerGroup,
		Simulation::PARAMS.maxVisNeuronsPerGroup);
}

int BrainGenome::GetNumInternalNeuralGroups()
{
	return GetGene(BrainGenome::GENE_NUM_INTERNAL_NEURAL_GROUPS).AsInt(
		Simulation::PARAMS.minInternalNeuralGroups,
		Simulation::PARAMS.maxInternalNeuralGroups);
}
