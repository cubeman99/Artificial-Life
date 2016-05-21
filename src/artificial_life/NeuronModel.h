#ifndef _NEURON_MODEL_H_
#define _NEURON_MODEL_H_

#include <vector>

	
float Sigmoid(float x, float slope);


//-----------------------------------------------------------------------------
// Neuron
//-----------------------------------------------------------------------------

// NOTE: activation levels are not maintained in the neuron struct
// so that after the new activation levels are computed, the old
// and new blocks of memory can simply be repointered rather than
// copied.

struct Neuron
{
	float	bias;
	float	tau; // ???
	int		startSynapse;
	int		endSynapse;
};


//-----------------------------------------------------------------------------
// Neuron Attributes
//-----------------------------------------------------------------------------

struct NeuronAttrs
{
	float bias;
	float tau; // UNUSED

	NeuronAttrs()
		: bias(0.0f)
		, tau(0.0f)
	{}

	NeuronAttrs(float bias, float tau = 0.0f)
		: bias(bias)
		, tau(tau)
	{}
};


//-----------------------------------------------------------------------------
// Synapse
//-----------------------------------------------------------------------------

struct Synapse
{
	float	efficacy; // > 0 for excitatory, < 0 for inhibitory
	float	learningRate;
	int		fromNeuron;
	int		toNeuron;
};


//-----------------------------------------------------------------------------
// Neuron Model
//-----------------------------------------------------------------------------

class NeuronModel
{
public:
	NeuronModel();
	NeuronModel(const NeuronModel& copy);
	~NeuronModel();

	void operator =(const NeuronModel& copy);

	struct Configuration
	{
		float maxBias;
		
		bool outputSynapseLearning;
		bool synapseFromOutputNeurons;

		long numPrebirthCycles;
		float sigmoidSlope;		// Higher numbers result in a steeper sigmoid curve.
		float maxWeight;		// Max weight for synapses.
		float initMaxWeight;
		float minLearningRate;
		float maxLearningRate;
		float decayRate;		// How much neuron activations decay per time step.
		short minWin;
		//short retinaWidth;
		//short retinaHeight;
		float maxSynapse2Energy; // (amount if all synapses usable)
		float maxNeuron2Energy;
	};

	struct Dimensions
	{
		Dimensions()
			: numInputNeurons(0)
			, numOutputNeurons(0)
			, numNeurons(0)
			, numSynapses(0)
		{}
		
		// Neuron's are stored in the following order:
		// Input, output, internal

		int numInputNeurons;
		int numOutputNeurons;
		int numNeurons;
		long numSynapses;

		inline int GetInputNeuronsBegin()		const { return 0; }
		inline int GetInputNeuronsEnd()			const { return numInputNeurons; }
		inline int GetOutputNeuronsBegin()		const { return numInputNeurons; }
		inline int GetOutputNeuronsEnd()		const { return numInputNeurons + numOutputNeurons; }
		inline int GetInternalNeuronsBegin()	const { return numInputNeurons + numOutputNeurons; }
		inline int GetInternalNeuronsEnd()		const { return numNeurons; }

		
		inline int GetNonInputNeuronsBegin()	const { return numInputNeurons; }
		inline int GetNonInputNeuronsEnd()		const { return numNeurons; }

		inline int GetNumNonInputNeurons()		const { return numNeurons - numInputNeurons; }
	};


	void Init(const Dimensions& dimensions, float initialActivation = 0.0f);
	void SetNeuron(int index, const NeuronAttrs& attributes, int startSynapses, int endSynapses);
	void SetSynapse(int index, int fromNeuron, int toNeuron, float efficacy, float learningRate);
	void Update();

	void SetNeuronActivation(int neuronIndex, float activation) { m_currNeuronActivations[neuronIndex] = activation; }
	float GetNeuronActivation(int neuronIndex) const { return m_currNeuronActivations[neuronIndex]; }
	
	float GetNeuronActivationPrev(int neuronIndex) const { return m_prevNeuronActivations[neuronIndex]; }


	const Neuron& GetNeuron(int neuronIndex) { return m_neurons[neuronIndex]; }
	const Synapse& GetSynapse(int synapseIndex) { return m_synapses[synapseIndex]; }
	
	void CreateNet();
	int GetNumWeights() const;
	std::vector<float> GetWeights() const;
	void SetWeights(const std::vector<float>& weights);
	std::vector<float> Update(const std::vector<float>& inputs);

	const Dimensions& GetDimensions() const { return m_dimensions; }
	void SetDimensions(const Dimensions& dims) { m_dimensions = dims; }

private:
	Configuration	m_config;
	Dimensions		m_dimensions;

	Neuron*		m_neurons;
	float*		m_prevNeuronActivations;
	float*		m_currNeuronActivations;
	Synapse*	m_synapses;
};


#endif // _NEURON_MODEL_H_