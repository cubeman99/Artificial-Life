#ifndef _BRAIN_H_
#define _BRAIN_H_

#include "NeuronModel.h"
#include "BrainGenome.h"
#include "util/Random.h"

class Brain
{
public:
	static struct Configuration
	{
		int numInputNeuralGroups;
		int numOutputNeuralGroups;
		int minVisNeuronsPerGroup;
		int maxVisNeuronsPerGroup;
		int minInternalNeuralGroups;
		int maxInternalNeuralGroups;
		int minENeuronsPerGroup;
		int maxENeuronsPerGroup;
		int minINeuronsPerGroup;
		int maxINeuronsPerGroup;
		int maxNeuronsPerGroup;

		float minConnectionDensity;
		float maxConnectionDensity;
		float minTopologicalDistortion;
		float maxTopologicalDistortion;
		float minSynapseLearningRate;
		float maxSynapseLearningRate;

		
		float maxBias;
		bool outputSynapseLearning;
		bool synapseFromOutputNeurons;
		long numPrebirthCycles;
		float logisticSlope;
		float maxWeight;
		float initMaxWeight;
		float minLearningRate;
		float maxLearningRate;
		float decayRate;
		//short minWin;
		//short retinaWidth;
		//short retinaHeight;
		//float maxsynapse2energy; // (amount if all synapses usable)
		//float maxneuron2energy;

		Configuration()
		{
			numInputNeuralGroups = 5; // red, green, blue, energy, random
			numOutputNeuralGroups = 7;
			minVisNeuronsPerGroup = 1;
			maxVisNeuronsPerGroup = 16;
			minInternalNeuralGroups = 1;
			maxInternalNeuralGroups = 5;
			minENeuronsPerGroup = 1;
			maxENeuronsPerGroup = 5;
			minINeuronsPerGroup = 1;
			maxINeuronsPerGroup = 5;
			//maxNeuronsPerGroup = ???

			minConnectionDensity = 0.0f;
			maxConnectionDensity = 1.0f;
			minTopologicalDistortion = 0.0f;
			minTopologicalDistortion = 1.0f;
			minSynapseLearningRate = 0.0f;
			minSynapseLearningRate = 0.1f;
		}
	};
	
public:
	Brain(BrainGenome* genome);
	~Brain();

	void Grow();
	void PreBirth();
	
	void GrowSynapses(int groupIndex_to,
					  int neuronCount_to,
					  float *remainder,
					  int* synapses_to,
					  int neuronLocalIndex_to,
					  int neuronIndex_to,
					  int* firstNeuron,
					  long &synapseCount_brain,
					  BrainGenome::SynapseType synapseType);
	

	NeuronModel* GetNeuralNet() { return m_neuronModel; }

	int GetNumNeuralGroups() const { return m_numGroups; }

private:
	static int NearestFreeNeuron(int iin, bool* used, int num, int exclude);

	NeuronModel*	m_neuronModel;
	int				m_numGroups;
	BrainGenome*	m_genome;

	RandomNumberGenerator m_rng;

};


#endif // _BRAIN_H_