#include "NeuronModel.h"
#include <AppLib/util/Random.h>
#include <AppLib/math/MathLib.h>


NeuronModel::Configuration NeuronModel::CONFIG;


NeuronModel::NeuronModel()
	: m_neurons(NULL)
	, m_prevNeuronActivations(NULL)
	, m_currNeuronActivations(NULL)
	, m_synapses(NULL)
{
	CONFIG.sigmoidSlope	= 1.0f;
	CONFIG.maxWeight	= 1.0f;
	CONFIG.decayRate	= 0.99f;
}

void NeuronModel::CopyFrom(const NeuronModel& copy)
{
	// Delete previously allocated buffers.
	if (m_neurons)
		delete [] m_neurons;
	if (m_prevNeuronActivations)
		delete [] m_prevNeuronActivations;
	if (m_currNeuronActivations)
		delete [] m_currNeuronActivations;
	if (m_synapses)
		delete [] m_synapses;

	m_dimensions			= copy.m_dimensions;
	m_neurons				= new Neuron[m_dimensions.numNeurons];
	m_prevNeuronActivations	= new float[m_dimensions.numNeurons];
	m_currNeuronActivations	= new float[m_dimensions.numNeurons];
	m_synapses				= new Synapse[m_dimensions.numSynapses];
		
	// Copy neurons and activations.
	for (int i = 0; i < m_dimensions.numNeurons; i++)
	{
		m_currNeuronActivations[i]	= copy.m_currNeuronActivations[i];
		m_prevNeuronActivations[i]	= copy.m_prevNeuronActivations[i];
		m_neurons[i]				= copy.m_neurons[i];
	}
	
	// Copy synapses.
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
	if (m_neurons)
		delete [] m_neurons;
	if (m_prevNeuronActivations)
		delete [] m_prevNeuronActivations;
	if (m_currNeuronActivations)
		delete [] m_currNeuronActivations;
	if (m_synapses)
		delete [] m_synapses;

	m_dimensions			= dimensions;
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

	//-----------------------------------------------------------------------------
	// Swap the prev and curr activation arrays.

	float* tempActivations = m_currNeuronActivations;
	m_currNeuronActivations = m_prevNeuronActivations;
	m_prevNeuronActivations = tempActivations;

	//-----------------------------------------------------------------------------
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
		m_currNeuronActivations[i] = Sigmoid(activation, CONFIG.sigmoidSlope);
	}
	
	//-----------------------------------------------------------------------------
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
		m_currNeuronActivations[i] = Sigmoid(activation, CONFIG.sigmoidSlope);
	}
	
	//-----------------------------------------------------------------------------
	// Update learning for all synapses.

	for (k = 0; k < m_dimensions.numSynapses; k++)
	{
		Synapse& synapse = m_synapses[k];

		// Hebbian learning.
		float efficacy = synapse.efficacy + synapse.learningRate
			* (m_currNeuronActivations[synapse.toNeuron] - 0.5f)
			* (m_prevNeuronActivations[synapse.fromNeuron] - 0.5f);
				
		// Gradually decay synapse efficacy.
        if (fabs(efficacy) > (0.5f * CONFIG.maxWeight))
        {
            efficacy *= 1.0f - (1.0f - CONFIG.decayRate) *
                (fabs(efficacy) - 0.5f * CONFIG.maxWeight) / (0.5f * CONFIG.maxWeight);
            if (efficacy > CONFIG.maxWeight)
                efficacy = CONFIG.maxWeight;
            else if (efficacy < -CONFIG.maxWeight)
                efficacy = -CONFIG.maxWeight;
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

float NeuronModel::Sigmoid(float x, float slope)
{
    return (1.0f / (1.0f + expf(-x * slope)));
}
