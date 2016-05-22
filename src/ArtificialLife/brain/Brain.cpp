#include "Brain.h"
#include <ArtificialLife/Simulation.h>
#include <AppLib/math/MathLib.h>
#include <AppLib/util/Random.h>
#include <assert.h>


Brain::Brain(BrainGenome* genome)
	: m_genome(genome)
	, m_numGroups(1)
{
	m_neuronModel = new NeuronModel();
}

Brain::~Brain()
{
	delete m_neuronModel; m_neuronModel = NULL;
}


// Grow the brain from its genome.
void Brain::Grow()
{
	//Random::Seed(0);
	m_rng.SetSeed(10);
	//m_rng.SeedTime();

	//-----------------------------------------------------------------------------
	// Decode the genome.

	int numRedNeurons			= m_genome->GetNumRedNeurons();
	int numGreenNeurons			= m_genome->GetNumGreenNeurons();
	int numBlueNeurons			= m_genome->GetNumBlueNeurons();
	int numInternalNeuralGroups	= m_genome->GetNumInternalNeuralGroups();

	int numNeuralGroups = numInternalNeuralGroups + Simulation::PARAMS.numInputNeurGroups + Simulation::PARAMS.numOutputNeurGroups;

	m_numGroups = numNeuralGroups;

	int* firstENeuron = new int[numNeuralGroups];
	int* firstINeuron = new int[numNeuralGroups];
	float* eeRemainder = new float[numNeuralGroups];
	float* eiRemainder = new float[numNeuralGroups];
	float* iiRemainder = new float[numNeuralGroups];
	float* ieRemainder = new float[numNeuralGroups];
	
	int* eeSynapseCounter = new int[numNeuralGroups];
	int* eiSynapseCounter = new int[numNeuralGroups];
	int* iiSynapseCounter = new int[numNeuralGroups];
	int* ieSynapseCounter = new int[numNeuralGroups];

	NeuronModel::Dimensions dim;
	dim.numInputNeurons		= numRedNeurons + numGreenNeurons + numBlueNeurons + 2;
	dim.numOutputNeurons	= Simulation::PARAMS.numOutputNeurGroups;
	dim.numSynapses			= 0;
	
	//-----------------------------------------------------------------------------
	// Configure Input/Output Neurons

	// Remember, groups are stored in the following order:
	// Input, Output, Internal.

	// The three vision colors have their own groups.
	// Every other input neuron is its own group.
	// 5 input groups.

	// Each output neuron is its own group.
	// 4 output groups. (7 in PolyWorld)

	int neuronIndex = 0;
	int numInOutGroups = Simulation::PARAMS.numInputNeurGroups + Simulation::PARAMS.numOutputNeurGroups;

	for (int group = 0; group < numInOutGroups; group++)
	{
		int numNeurons = m_genome->GetNeuronCount(NEURON_TYPE_EXCITATORY, group);
		
		firstENeuron[group] = neuronIndex; // In/out neuron groups don't distinguish inhibitory from excitatory.
		firstINeuron[group] = neuronIndex;

		neuronIndex += numNeurons;
	}
	
	//-----------------------------------------------------------------------------
	// Configure Internal Neuron Groups

	int numNonInputNeurons = dim.numOutputNeurons;

	for (int group = numInOutGroups; group < numNeuralGroups; group++)
	{
		BrainGenome::NeurGroupInfo groupInfo = m_genome->GetGroupInfo(group);
		
		firstENeuron[group] = dim.numInputNeurons + numNonInputNeurons;
		numNonInputNeurons += groupInfo.numExcitatoryNeurons;
		firstINeuron[group] = dim.numInputNeurons + numNonInputNeurons;
		numNonInputNeurons += groupInfo.numInhibitoryNeurons;
	}
	
	dim.numNeurons = dim.numInputNeurons + numNonInputNeurons;

	//-----------------------------------------------------------------------------
	// Count synapses.

	// Synapses from (all groups) to (non input groups).

	dim.numSynapses = 0;

	// For each non input group.
	for (int i = Simulation::PARAMS.numInputNeurGroups; i < numNeuralGroups; i++)
	{
		// For each group.
		for (int j = 0; j < numNeuralGroups; j++)
		{
			// Add the synapses from group j (any group) to group i (non-input group).
			dim.numSynapses += m_genome->GetSynapseCount(j, i);
		}
	}
		
	//-----------------------------------------------------------------------------
	// Allocate the Neural Network.

	// Now that we have counted the number of neurons and
	// synapses, we can allocate space for our network.
	m_neuronModel->Init(dim);

	//-----------------------------------------------------------------------------
	// Initialize input neuron activations.

	neuronIndex = 0;

	NeuronAttrs neuronAttrs;
	neuronAttrs.bias = 0.0f;
	neuronAttrs.tau = 0.0f;

	for (int group = 0; group < Simulation::PARAMS.numInputNeurGroups; group++)
	{
		int numNeurons = m_genome->GetNeuronCount(NEURON_TYPE_EXCITATORY, group);

		for (int j = 0; j < numNeurons; j++)
		{
			m_neuronModel->SetNeuron(neuronIndex++, neuronAttrs, -1, -1);
		}
	}
	

	//-----------------------------------------------------------------------------
	// Grow synapses.
	
	long synapseCounter = 0;

	// For each non input group.
	for (int groupTo = Simulation::PARAMS.numInputNeurGroups; groupTo < numNeuralGroups; groupTo++)
	{
		int synapseCounterBegin = synapseCounter;
		neuronAttrs.bias = m_genome->GetGroupGene(BrainGenome::GENE_INITIAL_BIAS, groupTo)
			.AsFloat(-Simulation::PARAMS.maxBias, Simulation::PARAMS.maxBias);

		// Zero all the synapse remainders.
		for (int groupFrom = 0; groupFrom < numNeuralGroups; groupFrom++)
		{
			eeRemainder[groupFrom] = 0.0f;
			eiRemainder[groupFrom] = 0.0f;
			iiRemainder[groupFrom] = 0.0f;
			ieRemainder[groupFrom] = 0.0f;
			eeSynapseCounter[groupFrom] = 0;
			eiSynapseCounter[groupFrom] = 0;
			iiSynapseCounter[groupFrom] = 0;
			ieSynapseCounter[groupFrom] = 0;
		}

		// Setup excitatory neurons for this group.
		int neuronCountTo = m_genome->GetNeuronCount(NEURON_TYPE_EXCITATORY, groupTo);
		
		for (int neuronLocalIndexTo = 0; neuronLocalIndexTo < neuronCountTo; neuronLocalIndexTo++)
		{
            int neuronIndexTo = firstENeuron[groupTo] + neuronLocalIndexTo;
			int startSynapse = synapseCounter;

			GrowSynapses(groupTo,
						 neuronCountTo,
						 eeRemainder,
						 eeSynapseCounter,
						 neuronLocalIndexTo,
						 neuronIndexTo,
						 firstENeuron,
						 synapseCounter,
						 BrainGenome::SYNAPSE_EE);
			GrowSynapses(groupTo,
						 neuronCountTo,
						 ieRemainder,
						 ieSynapseCounter,
						 neuronLocalIndexTo,
						 neuronIndexTo,
						 firstINeuron,
						 synapseCounter,
						 BrainGenome::SYNAPSE_IE);
			
			m_neuronModel->SetNeuron(neuronIndexTo, neuronAttrs, startSynapse, synapseCounter); 
		}

		// Setup inhibitory neurons for this group.
		neuronCountTo = m_genome->GetNeuronCount(NEURON_TYPE_INHIBITORY, groupTo);
		if (m_genome->IsOutputGroup(groupTo))
			neuronCountTo = 0;

		for (int neuronLocalIndexTo = 0; neuronLocalIndexTo < neuronCountTo; neuronLocalIndexTo++)
		{
            int neuronIndexTo = firstINeuron[groupTo] + neuronLocalIndexTo;
			int startSynapse = synapseCounter;

			GrowSynapses(groupTo,
						 neuronCountTo,
						 iiRemainder,
						 iiSynapseCounter,
						 neuronLocalIndexTo,
						 neuronIndexTo,
 						 firstINeuron,
						 synapseCounter,
						 BrainGenome::SYNAPSE_II);
			GrowSynapses(groupTo,
						 neuronCountTo,
						 eiRemainder,
						 eiSynapseCounter,
						 neuronLocalIndexTo,
						 neuronIndexTo,
						 firstENeuron,
						 synapseCounter,
						 BrainGenome::SYNAPSE_EI);
			
			m_neuronModel->SetNeuron(neuronIndexTo, neuronAttrs, startSynapse, synapseCounter); 
		}
	}

	if (synapseCounter < dim.numSynapses)
	{
		dim.numSynapses = synapseCounter;
		m_neuronModel->SetDimensions(dim);
	}
	else if (synapseCounter > dim.numSynapses)
	{
		std::cout << "ERROR: incorrect number of synapses!" << std::endl;
	}

	//-----------------------------------------------------------------------------
	// Cleanup.

	delete [] firstENeuron;
	delete [] firstINeuron;
	delete [] eeRemainder;
	delete [] eiRemainder;
	delete [] iiRemainder;
	delete [] ieRemainder;
	delete [] eeSynapseCounter;
	delete [] eiSynapseCounter;
	delete [] iiSynapseCounter;
	delete [] ieSynapseCounter;
}


// To-Group index
// Neuron Count of To-Group
// Remainer array for neuron groups
// Neuron Local index
// 

// Grow synapses to a neuron.
void Brain::GrowSynapses(int groupTo,
						 int neuronCount_to,
						 float *remainder,
						 int* synapses_to,
						 int neuronLocalIndex_to,
						 int neuronIndex_to, // Grow synapses to this neuron.
						 int* firstNeuron,
						 long &synapseCounter,
						 BrainGenome::SynapseType synapseType)
{
	const float initminweight = 0.0f;

	NeuronType synapseNeuronType_to = BrainGenome::GetSynapseNeuronType_To(synapseType);
	NeuronType synapseNeuronType_from = BrainGenome::GetSynapseNeuronType_From(synapseType);
	
	// For each neuron group.
	for (int groupFrom = 0; groupFrom < m_numGroups; groupFrom++)
	{
		// Grow synapses from this group's neurons (of the type specified in the synapse type's from-neuron-type).

		BrainGenome::NeurGroupSynapseInfo synapseInfo =
			m_genome->GetSynapseInfo(groupFrom, groupTo, synapseType);

		// Number of neurons in groupFrom.
		int neuronCount_from = m_genome->GetNeuronCount(synapseNeuronType_from, groupFrom);

		// Number of synapses from groupFrom to groupTo.
		int synapseCount_fromto = m_genome->GetSynapseCount(groupFrom, groupTo, synapseType);
		
		// Number of synapses per neuron in groupTo coming from groupFrom.
		float numSynapsesPerNeuron = (float) synapseCount_fromto / (float) neuronCount_to;

		// Number of synapses from groupFrom going to the target neuron.
		// Keep a running remainder, because connection density doesn't convert to integer numbers-of-neurons perfectly.
		int synapseCount_new = (int) (numSynapsesPerNeuron + remainder[groupFrom] + 1.e-5f);
		remainder[groupFrom] += numSynapsesPerNeuron - synapseCount_new;
		synapses_to[groupFrom] += synapseCount_new;

		if (neuronLocalIndex_to == neuronCount_to - 1)
		{
			if (synapses_to[groupFrom] != m_genome->GetSynapseCount(groupFrom, groupTo, synapseType))
			{
				std::cout << "SYNAPSE MISMATCH\n";
				//synapseCount_new += synapseCount_fromto - synapses_to[groupFrom];
				//assert(synapseCount_fromto - synapses_to[groupFrom] == 1);
			}
		}

		// "Local Index" means that the index is relative to the start of the neuron type (I or E) within
		// the group as opposed to the entire neuron array.
		//int neuronLocalIndex_fromBase = (int) (((float) neuronLocalIndex_to / (float) neuronCount_to) * (float) neuronCount_from - (synapseCount_new * 0.5f));

		int neuronLocalIndex_fromBase = (int) (((float) neuronLocalIndex_to / ((float) neuronCount_to - 1.0f)) * (neuronCount_from - synapseCount_new));
		neuronLocalIndex_fromBase = Math::Clamp(neuronLocalIndex_fromBase, 0, neuronCount_from - synapseCount_new);
				
		bool* neuronsUsed = new bool[neuronCount_from];
		memset(neuronsUsed, 0, neuronCount_from);
		
		// Grow a certain number of synapses.
		for (int isyn = 0; isyn < synapseCount_new; isyn++)
		{
			// "Local Index" means that the index is relative to the start of the neuron type (I or E) within
			// the group as opposed to the entire neuron array.
			int neuronLocalIndex_from = neuronLocalIndex_fromBase + isyn;

			// Topological distortion will change the neuron connection order.
			if (m_rng.NextFloat() < synapseInfo.topologicalDistortion)
			{
				int distortion = Math::NInt(m_rng.NextFloat(-0.5f, 0.5f) *
					synapseInfo.topologicalDistortion * neuronCount_from);
				neuronLocalIndex_from += distortion;
				
				// Wrap around.
				if (neuronLocalIndex_from < 0)
					neuronLocalIndex_from += neuronCount_from;
				else if (neuronLocalIndex_from >= neuronCount_from)
					neuronLocalIndex_from -= neuronCount_from;
			}

			// Make sure this neuron isnt already connected to.
			if (firstNeuron[groupFrom] + neuronLocalIndex_from == neuronIndex_to || // same neuron
				neuronsUsed[neuronLocalIndex_from]) // already connected to this one
			{
				// Are we connecting this bunch of neurons to itself?
				if (groupTo == groupFrom && // same group
					(synapseNeuronType_from == synapseNeuronType_to || // same neuron type (I or E)
					m_genome->IsOutputGroup(groupTo)))
				{
					neuronLocalIndex_from = NearestFreeNeuron(neuronLocalIndex_from,
															  neuronsUsed,
															  neuronCount_from,
															  neuronLocalIndex_to);
				}
				else
				{
					neuronLocalIndex_from = NearestFreeNeuron(neuronLocalIndex_from,
															  neuronsUsed,
															  neuronCount_from,
															  neuronLocalIndex_from);
				}
			}
			
			// Mark this neuron as 'used'.
			neuronsUsed[neuronLocalIndex_from] = true;

			int neuronIndex_from = firstNeuron[groupFrom] + neuronLocalIndex_from;
			
			// We should never have a self-synapsing neuron.
			assert(neuronIndex_from != neuronIndex_to);
			
			// Randomize the synapse's initial weight (efficacy).
			float efficacy;
			if (synapseNeuronType_from == NeuronType::NEURON_TYPE_INHIBITORY)
				efficacy = Math::Min(-1.e-10f, -m_rng.NextFloat(initminweight, Simulation::PARAMS.initMaxWeight));
			else
				efficacy = m_rng.NextFloat(initminweight, Simulation::PARAMS.initMaxWeight);

			// Learning Rate. TODO: option to turn off output synapse learning.
			float learningRate = synapseInfo.synapseLearningRate;
			
			// Finally, configure the synapse in the neural-net.
			m_neuronModel->SetSynapse(synapseCounter,
									  neuronIndex_from,
									  neuronIndex_to,
									  efficacy,
									  learningRate);
			synapseCounter++;
		}
		

		delete [] neuronsUsed;
	}
}

int Brain::NearestFreeNeuron(int iin, bool* used, int num, int exclude)
{
    int iout;
    bool tideishigh;
    int hitide = iin;
    int lotide = iin;

    if (iin < num - 1)
    {
        iout = iin + 1;
        tideishigh = true;
    }
    else
    {
        iout = iin - 1;
        tideishigh = false;
    }

    while (used[iout] || iout == exclude)
    {
        if (tideishigh)
        {
            hitide = iout;
            if (lotide > 0)
            {
                iout = lotide - 1;
                tideishigh = false;
            }
            else if (hitide < num - 1)
                iout++;
        }
        else
        {
            lotide = iout;
            if (hitide < num - 1)
            {
                iout = hitide + 1;
                tideishigh = true;
            }
            else if (lotide > 0)
                iout--;
        }

        if (lotide == 0 && hitide == num - 1)
			assert(false); // NearestFreeNeuron search failed!!!
    }

    return iout;
}


void Brain::PreBirth()
{
	for (int i = 0; i < Simulation::PARAMS.numPrebirthCycles; i++)
	{
		// Feed randomized values to the neural net's inputs.
		int inputNeuronsBegin = m_neuronModel->GetDimensions().GetInputNeuronsBegin();
		int inputNeuronsEnd   = m_neuronModel->GetDimensions().GetInputNeuronsEnd();
		for (int i = inputNeuronsBegin; i < inputNeuronsEnd; i++)
		{
			m_neuronModel->SetNeuronActivation(i, Random::NextFloat());
		}

		// Update the net.
		m_neuronModel->Update();
	}
}

