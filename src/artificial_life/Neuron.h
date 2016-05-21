#ifndef _NEURON_H_
#define _NEURON_H_

#include <ctime>
#include <random>
#include <string>
#include "util/Random.h"

namespace SweeperAI
{

class NeuronLayer;
class NeuralNet;

class Neuron
{
public:
	friend class NeuronLayer;
	friend class NeuralNet;

public:
	Neuron(int numInputs);

private:
	std::vector<float> m_weights;
	int m_numInputs;
};


class NeuronLayer
{
public:
	friend class NeuralNet;

public:
	NeuronLayer(int numNeurons, int numInputsPerNeuron);

private:
	std::vector<Neuron> m_neurons;
};



class NeuralNet
{
public:
	NeuralNet();

	void CreateNet();

	int GetNumWeights();
	std::vector<float> GetWeights();
	void SetWeights(const std::vector<float>& weights);

	std::vector<float> Update(const std::vector<float>& inputs);

	inline float Sigmoid(float activation, float response)
	{
		return (1.0f / (1.0f + expf(-activation / response)));
	}

private:
	std::vector<NeuronLayer> m_layers;

	int m_numInputs;
	int m_numOutputs;
	int m_numHiddenLayers;
	int m_numNeuronsPerHiddenLayer;
};


};

#endif // _NEURON_H_