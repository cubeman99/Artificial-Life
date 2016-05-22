
#include "artificial_life/SimulationApp.h"
#include "artificial_life/NeuronModel.h"
#include "artificial_life/Genome.h"
#include "artificial_life/BrainGenome.h"
#include "artificial_life/Brain.h"
#include "graphics_tests/GraphicsTestApp.h"
#include "math/MathLib.h"
#include <iostream>

using namespace std;


void PrintGenome(const char* genomeName, BrainGenome* genome)
{

	//printf("%s = ", genomeName);
	//for (int i = 0; i < genome->GetDataSize(); i++)
		//printf("%.2X ", genome->GetData()[i]);
	//printf("\n");
	
	cout << "Size = " << genome->GetGene(BrainGenome::GENE_SIZE).AsFloat() << endl;
	cout << "Strength = " << genome->GetGene(BrainGenome::GENE_STRENGTH).AsFloat() << endl;
	cout << "Internal groups = " << genome->GetGene(BrainGenome::GENE_NUM_INTERNAL_NEURAL_GROUPS).AsInt(1, 5) << endl;

	int numGroups = Simulation::PARAMS.numInputNeurGroups + Simulation::PARAMS.numOutputNeurGroups + Simulation::PARAMS.maxInternalNeuralGroups;

	for (int i = 0; i < numGroups; i++)
	{
		BrainGenome::NeurGroupInfo groupInfo = genome->GetGroupInfo(i);

		cout << "Group " << i << " (" << groupInfo.numExcitatoryNeurons << "e, " << groupInfo.numInhibitoryNeurons << "i)" << endl;

		for (int j = 0; j < numGroups; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				const char* syn = "??";
				if (k == BrainGenome::SYNAPSE_EE)
					syn = "EE";
				if (k == BrainGenome::SYNAPSE_EI)
					syn = "EI";
				if (k == BrainGenome::SYNAPSE_II)
					syn = "II";
				if (k == BrainGenome::SYNAPSE_IE)
					syn = "IE";
				BrainGenome::NeurGroupSynapseInfo synapseInfo = genome->GetSynapseInfo(i, j, (BrainGenome::SynapseType) k);

				printf("%d,%d,%s - CD=%f, TD=%f, LR=%f\n", i, j, syn,
					synapseInfo.connectionDensity,
					synapseInfo.topologicalDistortion,
					synapseInfo.synapseLearningRate);
				
			}
		}
	}
}

int main(int argc, char** argv)
{
	Random::SeedTime();

	/*
	for (int i = 0; i < 100000; i++)
	{
		Agent* agent = new Agent(NULL);
		delete agent;
		//BrainGenome* genome = new BrainGenome();
		//genome->Randomize();
		//Brain* brain = new Brain(genome);
		//brain->PreBirth();
		cout << i << endl;
		//delete brain;
		//delete genome;
	}
	system("pause");
	return 0;
	*/

	/*
	int size = 24;

	BrainGenome* g1 = new BrainGenome();
	//g1->InitSize(size);
	g1->Randomize();
	//PrintGenome("G1", g1);
	//cout << endl;

	BrainGenome* g2 = new BrainGenome();
	//g2->InitSize(size);
	g2->Randomize();
	//PrintGenome("G2", g2);
	//cout << endl;
	
	BrainGenome* g3 = new BrainGenome();
	g3->Crossover(g1, g2);
	g3->Mutate(0.03f);
	//PrintGenome("G3", g3);
	//cout << endl;

	Brain brain(g1);
	brain.PreBirth();

	NeuronModel* neuralNet = brain.GetNeuralNet();

	int ouputNeuronsBegin = neuralNet->GetDimensions().GetOutputNeuronsBegin();
	int outputNeuronsEnd = neuralNet->GetDimensions().GetOutputNeuronsEnd();
	for (int i = ouputNeuronsBegin; i < outputNeuronsEnd; i++)
	{
		float activation = neuralNet->GetNeuronActivation(i);
		cout << "Neuron " << i << " = " << activation << endl;
	}


	system("pause");
	return 0;
	*/

	/*
	NeuronModel neuronModel;
	neuronModel.CreateNet();
	
	int numInputs = 2;
	int numOutputs = 2;
	int numHiddenLayers = 1;
	int numNeuronsPerHiddenLayer = 6;

	NeuronModel::Dimensions dim;
	dim.numInputNeurons  = numInputs;
	dim.numOutputNeurons = numOutputs;
	dim.numNeurons       = numInputs + numOutputs + (numHiddenLayers * numNeuronsPerHiddenLayer);

	dim.numSynapses =
		(numInputs  * numNeuronsPerHiddenLayer) +
		(numOutputs * numNeuronsPerHiddenLayer) +
		((numNeuronsPerHiddenLayer * numNeuronsPerHiddenLayer) * (numHiddenLayers - 1));
	
	int numWeights = neuronModel.GetNumWeights();

	cout << "numWeights = " << numWeights << endl;

	neuronModel.SetNeuron(0, NeuronAttrs(0.0f), -1, -1);
	neuronModel.SetNeuron(0, NeuronAttrs(0.0f), -1, -1);

	float in1 = Random::NextFloatClamped();
	float in2 = Random::NextFloatClamped();
	neuronModel.SetNeuronActivation(dim.GetInputNeuronsBegin() + 0, in1);
	neuronModel.SetNeuronActivation(dim.GetInputNeuronsBegin() + 1, in2);
	neuronModel.Update();
	neuronModel.SetNeuronActivation(dim.GetInputNeuronsBegin() + 0, in1);
	neuronModel.SetNeuronActivation(dim.GetInputNeuronsBegin() + 1, in2);
	cout << "Input1 = " << in1 << endl;
	cout << "Input2 = " << in2 << endl;

	for (int i = 0; i < 5; i++)
	{

		cout << "\nUpdating network...\n" << endl;
		neuronModel.Update();

		float out1 = neuronModel.GetNeuronActivation(dim.GetOutputNeuronsBegin() + 0);
		float out2 = neuronModel.GetNeuronActivation(dim.GetOutputNeuronsBegin() + 1);
		cout << "Out1 = " << out1 << endl;
		cout << "Out2 = " << out2<< endl;
	}

	//system("pause");
	//return 0;
	*/

	const char* title = "Artificial Life - David Jordan (2016)";
	int width  = 1100;
	int height = 800;

	//circle_fitting::CircleAIApp application;
	//GraphicsTestApp application;
	SimulationApp application;
	if (!application.Initialize(title, width, height))
		return 1;

	application.Run();

	return 0;
}
