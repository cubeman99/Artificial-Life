#include "Neuron.h"

namespace SweeperAI
{

Neuron::Neuron(int numInputs)
	: m_numInputs(numInputs + 1)
{
	// Create random weights.
	// The extra weight at the end is for the bias.
	for (int i = 0; i < numInputs + 1; i++)
		m_weights.push_back(Random::NextFloatClamped());
}


//-----------------------------------------------------------------------------


NeuronLayer::NeuronLayer(int numNeurons, int numInputsPerNeuron)
{
	for (int i = 0; i < numNeurons; i++)
	{
		m_neurons.push_back(Neuron(numInputsPerNeuron));
	}
}


//-----------------------------------------------------------------------------


NeuralNet::NeuralNet()
{
	m_numInputs					= 2;//4;
	m_numOutputs				= 2;
	m_numHiddenLayers			= 1;
	m_numNeuronsPerHiddenLayer	= 6;

	CreateNet();
}

void NeuralNet::CreateNet()
{
	// Create the layers of the network.
	if (m_numHiddenLayers > 0)
	{
		// Create first hidden layer.
		m_layers.push_back(NeuronLayer(m_numNeuronsPerHiddenLayer, m_numInputs));

		for (int i = 0; i < m_numHiddenLayers - 1; ++i)
		{
			m_layers.push_back(NeuronLayer(
				m_numNeuronsPerHiddenLayer,
				m_numNeuronsPerHiddenLayer));
		}

		// Create output layer.
		m_layers.push_back(NeuronLayer(m_numOutputs, m_numNeuronsPerHiddenLayer));
	}

	else
	{
		// Create a single output layer.
		m_layers.push_back(NeuronLayer(m_numOutputs, m_numInputs));
	}
}

int NeuralNet::GetNumWeights()
{
	int numWeights = 0;

	// For each layer.
	for (int i = 0; i < m_numHiddenLayers + 1; i++)
	{
		// For each neuron.
		for (int j = 0; j < (int) m_layers[i].m_neurons.size(); j++)
			numWeights += m_layers[i].m_neurons[j].m_numInputs;
	}

	return numWeights;
}

std::vector<float> NeuralNet::GetWeights()
{
	std::vector<float> weights;
	
	// For each layer.
	for (int i = 0; i < m_numHiddenLayers + 1; i++)
	{
		// For each neuron.
		for (int j = 0; j < (int) m_layers[i].m_neurons.size(); j++)
		{
			// For each input (plus the bias).
			for (int k = 0; k < m_layers[i].m_neurons[j].m_numInputs; k++)
				weights.push_back(m_layers[i].m_neurons[j].m_weights[k]);
		}
	}

	return weights;
}

void NeuralNet::SetWeights(const std::vector<float>& weights)
{
	int weightIndex = 0;

	// For each layer.
	for (int i = 0; i < m_numHiddenLayers + 1; i++)
	{
		// For each neuron.
		for (int j = 0; j < (int) m_layers[i].m_neurons.size(); j++)
		{
			// For each input (plus the bias).
			for (int k = 0; k < m_layers[i].m_neurons[j].m_numInputs;  k++)
				m_layers[i].m_neurons[j].m_weights[k] = weights[weightIndex++];
		}
	}
}

std::vector<float> NeuralNet::Update(const std::vector<float>& firstInputs)
{
	std::vector<float> inputs = firstInputs;
	std::vector<float> outputs;
	
	// Params:
	float bias = -1.0f;
	float activationResponse = 1.0f;
	
	// For each layer.
	for (int i = 0; i < m_numHiddenLayers + 1; i++)
	{
		// The outputs of the previous layer are the inputs to this layer.
		if (i > 0)
			inputs = outputs;

		outputs.clear();

		// For each neuron, sum the (inputs * corresponding weights).
		// Throw the total at our sigmoid function to get the output.
		for (int j = 0; j < (int) m_layers[i].m_neurons.size(); j++)
		{
			Neuron* neuron = &m_layers[i].m_neurons[j];
			int	numInputs = neuron->m_numInputs;

			float netInput = 0.0f;
			
			// Sum the weights * inputs.
			for (int k = 0; k < numInputs - 1;  k++)
				netInput += neuron->m_weights[k] * inputs[k];

			// Add in the bias.
			netInput += neuron->m_weights[numInputs - 1] * bias;
			
			// We can store the outputs from each layer as we generate them. 
			// The combined activation is first filtered through the sigmoid 
			// function.
			outputs.push_back(Sigmoid(netInput, activationResponse));
		}
	}

	return outputs;
}


};
