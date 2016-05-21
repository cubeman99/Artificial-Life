#include "NeuronModel.h"
#include "util/Random.h"
#include "math/MathLib.h"


	
float Sigmoid(float x, float slope)
{
    return (1.0f / (1.0f + expf(-x * slope)));
}



NeuronModel::NeuronModel()
	: m_neurons(NULL)
	, m_prevNeuronActivations(NULL)
	, m_currNeuronActivations(NULL)
	, m_synapses(NULL)
{
	m_config.sigmoidSlope = 1.0f;
	m_config.maxWeight = 1.0f;
	m_config.decayRate = 0.1f;
}

NeuronModel::NeuronModel(const NeuronModel& copy)
	: m_config(copy.m_config)
	, m_dimensions(copy.m_dimensions)
{
	m_neurons				= new Neuron[m_dimensions.numNeurons];
	m_prevNeuronActivations	= new float[m_dimensions.numNeurons];
	m_currNeuronActivations	= new float[m_dimensions.numNeurons];
	m_synapses				= new Synapse[m_dimensions.numSynapses];
		
	for (int i = 0; i < m_dimensions.numNeurons; i++)
	{
		m_currNeuronActivations[i] = copy.m_currNeuronActivations[i];
		m_prevNeuronActivations[i] = copy.m_prevNeuronActivations[i];
		m_neurons[i]  = copy.m_neurons[i];
	}
	
	for (long i = 0; i < m_dimensions.numSynapses; i++)
	{
		m_synapses[i] = copy.m_synapses[i];
	}
}


void NeuronModel::operator =(const NeuronModel& copy)
{
	if (m_neurons)
		delete [] m_neurons;
	if (m_prevNeuronActivations)
		delete [] m_prevNeuronActivations;
	if (m_currNeuronActivations)
		delete [] m_currNeuronActivations;
	if (m_synapses)
		delete [] m_synapses;

	m_config = copy.m_config;
	m_dimensions = copy.m_dimensions;

	m_neurons				= new Neuron[m_dimensions.numNeurons];
	m_prevNeuronActivations	= new float[m_dimensions.numNeurons];
	m_currNeuronActivations	= new float[m_dimensions.numNeurons];
	m_synapses				= new Synapse[m_dimensions.numSynapses];
		
	for (int i = 0; i < m_dimensions.numNeurons; i++)
	{
		m_currNeuronActivations[i] = copy.m_currNeuronActivations[i];
		m_prevNeuronActivations[i] = copy.m_prevNeuronActivations[i];
		m_neurons[i]  = copy.m_neurons[i];
	}
	
	for (long i = 0; i < m_dimensions.numSynapses; i++)
	{
		m_synapses[i] = copy.m_synapses[i];
	}
}

NeuronModel::~NeuronModel()
{
	delete [] m_neurons; m_neurons = NULL;
	delete [] m_prevNeuronActivations; m_prevNeuronActivations = NULL;
	delete [] m_currNeuronActivations; m_currNeuronActivations = NULL;
	delete [] m_synapses; m_synapses = NULL;
}

void NeuronModel::Init(const Dimensions& dimensions, float initialActivation)
{
	m_dimensions = dimensions;
	
	if (m_neurons)
		delete [] m_neurons;
	if (m_prevNeuronActivations)
		delete [] m_prevNeuronActivations;
	if (m_currNeuronActivations)
		delete [] m_currNeuronActivations;
	if (m_synapses)
		delete [] m_synapses;

	m_neurons				= new Neuron[m_dimensions.numNeurons];
	m_prevNeuronActivations	= new float[m_dimensions.numNeurons];
	m_currNeuronActivations	= new float[m_dimensions.numNeurons];
	m_synapses				= new Synapse[m_dimensions.numSynapses];

	for (int i = 0; i < m_dimensions.numNeurons; i++)
	{
		m_currNeuronActivations[i] = initialActivation;
		m_prevNeuronActivations[i] = initialActivation;
	}
}

void NeuronModel::SetNeuron(int index, const NeuronAttrs& attributes, int startSynapse, int endSynapse)
{
	m_neurons[index].bias			= attributes.bias;
	m_neurons[index].tau			= attributes.tau;
	m_neurons[index].startSynapse	= startSynapse;
	m_neurons[index].endSynapse		= endSynapse;
}

void NeuronModel::SetSynapse(int index, int fromNeuron, int toNeuron, float efficacy, float learningRate)
{
	m_synapses[index].fromNeuron	= fromNeuron;
	m_synapses[index].toNeuron		= toNeuron;
	m_synapses[index].efficacy		= efficacy;
	m_synapses[index].learningRate	= learningRate;
}

void NeuronModel::Update()
{
	long k;

	// Swap the prev and curr activation arrays.
	float* tempActivations = m_currNeuronActivations;
	m_currNeuronActivations = m_prevNeuronActivations;
	m_prevNeuronActivations = tempActivations;

	// Update output neurons.
	for (int i = m_dimensions.GetOutputNeuronsBegin(); i < m_dimensions.GetOutputNeuronsEnd(); i++)
	{
		// Add in the bias term.
		float activation = m_neurons[i].bias;

		// Sum up the inputs to this neuron times their synapse weights (efficacies).
		for (k = m_neurons[i].startSynapse; k < m_neurons[i].endSynapse; k++)
		{
			activation += m_synapses[k].efficacy *
				m_prevNeuronActivations[m_synapses[k].fromNeuron];
		}

		// Apply the sigmoid function to the resulting activation.
		m_currNeuronActivations[i] = Sigmoid(activation, m_config.sigmoidSlope);
	}
	
	// Update internal neurons.
	for (int i = m_dimensions.GetInternalNeuronsBegin(); i < m_dimensions.GetInternalNeuronsEnd(); i++)
	{
		// Add in the bias term.
		float activation = m_neurons[i].bias;

		// Sum up the inputs to this neuron times their synapse weights (efficacies).
		for (k = m_neurons[i].startSynapse; k < m_neurons[i].endSynapse; k++)
		{
			activation += m_synapses[k].efficacy *
				m_prevNeuronActivations[m_synapses[k].fromNeuron];
		}

		// Apply the sigmoid function to the resulting activation.
		m_currNeuronActivations[i] = Sigmoid(activation, m_config.sigmoidSlope);
	}
	
	// Update learning for all synapses.
	for (k = 0; k < m_dimensions.numSynapses; k++)
	{
		Synapse& synapse = m_synapses[k];

		//synapse.learningRate = 0.05f;
		m_config.decayRate = 0.99f;

		// Hebbian learning.
		float efficacy = synapse.efficacy + synapse.learningRate
			* (m_currNeuronActivations[synapse.toNeuron] - 0.5f)
			* (m_prevNeuronActivations[synapse.fromNeuron] - 0.5f);
				
		// Gradually decay synapse efficacy.
        if (fabs(efficacy) > (0.5f * m_config.maxWeight))
        {
            efficacy *= 1.0f - (1.0f - m_config.decayRate) *
                (fabs(efficacy) - 0.5f * m_config.maxWeight) / (0.5f * m_config.maxWeight);
            if (efficacy > m_config.maxWeight)
                efficacy = m_config.maxWeight;
            else if (efficacy < -m_config.maxWeight)
                efficacy = -m_config.maxWeight;
        }
        else
        {
            // not strictly correct for this to be in an else clause,
            // but if lrate is reasonable, efficacy should never change
            // sign with a new magnitude greater than 0.5 * Brain::config.maxWeight
            if (synapse.learningRate >= 0.0f)  // excitatory
                efficacy = Math::Max(0.0f, efficacy);
            if (synapse.learningRate < 0.0f)  // inhibitory
                efficacy = Math::Min(-1.e-10f, efficacy);
        }
		
		synapse.efficacy = efficacy;
		
	}
}



void NeuronModel::CreateNet()
{
	int numInputs					= 4; //7 + 8;
	int numOutputs					= 2;
	int numHiddenLayers				= 1 + 1;
	int numNeuronsPerHiddenLayer	= 6;

	NeuronModel::Dimensions dim;
	dim.numInputNeurons  = numInputs;
	dim.numOutputNeurons = numOutputs;
	dim.numNeurons       = numInputs + numOutputs + (numHiddenLayers * numNeuronsPerHiddenLayer);

	dim.numSynapses =
		(numInputs  * numNeuronsPerHiddenLayer) +
		(numOutputs * numNeuronsPerHiddenLayer) +
		((numNeuronsPerHiddenLayer * numNeuronsPerHiddenLayer) * (numHiddenLayers - 1));
	
	Init(dim);
	
	int synapseIndex = 0;

	// Create the layers of the network.
	if (numHiddenLayers > 0)
	{
		int neuronIndex  = dim.GetInternalNeuronsBegin();

		// Setup input layer.
		for (int j = 0; j < numInputs; j++)
		{
			SetNeuron(dim.GetInputNeuronsBegin() + j,
				NeuronAttrs(0.0f), -1, -1);
		}

		// Create first hidden layer.
		for (int j = 0; j < numNeuronsPerHiddenLayer; j++)
		{
			SetNeuron(neuronIndex,
				NeuronAttrs(Random::NextFloatClamped()),
				synapseIndex, synapseIndex + numInputs);

			for (int k = 0; k < numInputs; k++)
			{
				SetSynapse(synapseIndex++, dim.GetInputNeuronsBegin() + k,
					neuronIndex, Random::NextFloatClamped(), 0.0f);
			}

			neuronIndex++;
		}

		// Create other hidden layers.
		for (int i = 1; i < numHiddenLayers; ++i)
		{
			for (int j = 0; j < numNeuronsPerHiddenLayer; j++)
			{
				SetNeuron(neuronIndex,
					NeuronAttrs(Random::NextFloatClamped()),
					synapseIndex, synapseIndex + numNeuronsPerHiddenLayer);
				
				for (int k = 0; k < numNeuronsPerHiddenLayer; k++)
				{
					SetSynapse(synapseIndex++, dim.GetInternalNeuronsBegin() +
						((i - 1) * numNeuronsPerHiddenLayer) + k,
						neuronIndex, Random::NextFloatClamped(), 0.0f);
				}

				neuronIndex++;
			}
		}

		// Create output layer.
		for (int j = 0; j < numOutputs; j++)
		{
			neuronIndex = dim.GetOutputNeuronsBegin() + j;

			SetNeuron(neuronIndex,
				NeuronAttrs(Random::NextFloatClamped()),
				synapseIndex, synapseIndex + numNeuronsPerHiddenLayer);

			for (int k = 0; k < numNeuronsPerHiddenLayer; k++)
			{
				SetSynapse(synapseIndex++, dim.GetInternalNeuronsBegin() +
					((numHiddenLayers - 1) * numNeuronsPerHiddenLayer) + k,
					neuronIndex, Random::NextFloatClamped(), 0.0f);
			}
		}

		for (int i = 0; i < m_dimensions.numSynapses; i++)
		{
			m_synapses[i].learningRate = Random::NextFloatClamped() * 0.1f;
		}
	}
}

int NeuronModel::GetNumWeights() const
{
	int numWeights = 0;
	for (int i = m_dimensions.GetNonInputNeuronsBegin(); i < m_dimensions.GetNonInputNeuronsEnd(); i++)
		numWeights += (m_neurons[i].endSynapse - m_neurons[i].startSynapse) + 1;
	return numWeights;
}

std::vector<float> NeuronModel::GetWeights() const
{
	std::vector<float> weights;
	
	for (int i = m_dimensions.GetNonInputNeuronsBegin(); i < m_dimensions.GetNonInputNeuronsEnd(); i++)
	{
		for (int k = m_neurons[i].startSynapse; k < m_neurons[i].endSynapse; k++)
			weights.push_back(m_synapses[k].efficacy);
		weights.push_back(m_neurons[i].bias);
	}

	return weights;
}

void NeuronModel::SetWeights(const std::vector<float>& weights)
{
	int weightIndex = 0;
	
	for (int i = m_dimensions.GetNonInputNeuronsBegin(); i < m_dimensions.GetNonInputNeuronsEnd(); i++)
	{
		for (int k = m_neurons[i].startSynapse; k < m_neurons[i].endSynapse; k++)
			m_synapses[k].efficacy = weights[weightIndex++];
		m_neurons[i].bias = weights[weightIndex++];
	}
}

std::vector<float> NeuronModel::Update(const std::vector<float>& inputs)
{
	// Set the inputs.
	for (int i = 0; i < m_dimensions.numInputNeurons; i++)
	{
		int neuronIndex = m_dimensions.GetInputNeuronsBegin() + i;
		m_currNeuronActivations[neuronIndex] = inputs[i];
		m_prevNeuronActivations[neuronIndex] = inputs[i];
	}

	// Update the network.
	Update();

	// Gather the outputs.
	std::vector<float> outputs;
	for (int i = m_dimensions.GetOutputNeuronsBegin(); i < m_dimensions.GetOutputNeuronsEnd(); i++)
		outputs.push_back(m_currNeuronActivations[i]);

	return outputs;
}
