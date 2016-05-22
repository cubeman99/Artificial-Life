#include "BrainRenderer.h"
#include <AppLib/math/MathLib.h>
#include <ArtificialLife/Simulation.h>


BrainRenderer::BrainRenderer()
{
}


void BrainRenderer::RenderBrain(Graphics* g, Agent* agent, const Vector2f& position)
{
	NeuronModel* neuralNet = agent->GetBrain()->GetNeuralNet();
	NeuronModel::Dimensions dims = neuralNet->GetDimensions();
		
	int numNonInputNeurons	= dims.GetNumNonInputNeurons();
	int numRedNeurons		= agent->GetGenome()->GetNumRedNeurons();
	int numGreenNeurons		= agent->GetGenome()->GetNumGreenNeurons();
	int numBlueNeurons		= agent->GetGenome()->GetNumBlueNeurons();
	int numVisionNeurons	= numRedNeurons + numGreenNeurons + numBlueNeurons;
	int numInputGroups		= Simulation::PARAMS.numInputNeurGroups;
	int numOutputGroups		= Simulation::PARAMS.numInputNeurGroups;

	Vector2f cellSize(10, 10);
	Vector2f boxSize(dims.numNeurons * cellSize.x,
					 numNonInputNeurons * cellSize.y);

	Color colorBackground(80, 80, 80);

	g->EnableCull(false);
	g->EnableDepthTest(false);
	g->ResetTransform();
	g->Translate(position - (boxSize * 0.5f));

	// Draw background rectangle.
	g->FillRect(Vector2f(0.0f, 0.0f),
		Vector2f((float) dims.numNeurons,
		(float) numNonInputNeurons) * cellSize,
		colorBackground);
	
	// Draw synapses and neuron activations.
	for (int i = 0; i < dims.numNeurons; i++)
	{
		auto neuron = neuralNet->GetNeuron(i);
		
		// Draw all the synapses going to this neuron (a single row in the connection matrix).
		for (int isyn = neuron.startSynapse; isyn < neuron.endSynapse; isyn++)
		{
			auto synapse = neuralNet->GetSynapse(isyn);
			auto fromNeuron = neuralNet->GetNeuron(synapse.fromNeuron);
				
			// Check if is an inhibitory and an excitatory connection to this neuron.
			int count = 0;
			for (int jsyn = neuron.startSynapse; jsyn < neuron.endSynapse; jsyn++)
			{
				if (neuralNet->GetSynapse(jsyn).fromNeuron == synapse.fromNeuron)
					count++;
			}

			bool excitatory = (synapse.efficacy >= 0.0f);

			// Determine the color of the synapse.
			float absEfficacy = Math::Abs(synapse.efficacy);
			Vector3f synapseColor = Vector3f::ZERO;
			if (excitatory)
				synapseColor.y = synapse.efficacy; // Green for excitatory neurons.
			else
				synapseColor.x = -synapse.efficacy; // Red for inhibitory neurons.

			// Draw the synapse square.
			for (int k = 0; k < 2; k++)
			{
				if (k == 0)
				{
					glBegin(GL_TRIANGLES);
					glColor3fv(synapseColor.data());	
				}
				else
				{
					glBegin(GL_LINE_LOOP);
					if (excitatory)
						glColor4ubv(Color::DARK_GREEN.data());
					else
						glColor4ubv(Color::DARK_RED.data());
				}

				if (count == 1)
				{
					if (k == 0)
					{
						glVertex2f((synapse.fromNeuron + 0) * cellSize.x, (synapse.toNeuron + 0 - dims.numInputNeurons) * cellSize.y);
						glVertex2f((synapse.fromNeuron + 1) * cellSize.x, (synapse.toNeuron + 0 - dims.numInputNeurons) * cellSize.y);
						glVertex2f((synapse.fromNeuron + 1) * cellSize.x, (synapse.toNeuron + 1 - dims.numInputNeurons) * cellSize.y);

						glVertex2f((synapse.fromNeuron + 0) * cellSize.x, (synapse.toNeuron + 0 - dims.numInputNeurons) * cellSize.y);
						glVertex2f((synapse.fromNeuron + 1) * cellSize.x, (synapse.toNeuron + 1 - dims.numInputNeurons) * cellSize.y);
						glVertex2f((synapse.fromNeuron + 0) * cellSize.x, (synapse.toNeuron + 1 - dims.numInputNeurons) * cellSize.y);
					}
					else
					{
						glVertex2f((synapse.fromNeuron + 0) * cellSize.x, (synapse.toNeuron + 0 - dims.numInputNeurons) * cellSize.y);
						glVertex2f((synapse.fromNeuron + 1) * cellSize.x, (synapse.toNeuron + 0 - dims.numInputNeurons) * cellSize.y);
						glVertex2f((synapse.fromNeuron + 1) * cellSize.x, (synapse.toNeuron + 1 - dims.numInputNeurons) * cellSize.y);
						glVertex2f((synapse.fromNeuron + 0) * cellSize.x, (synapse.toNeuron + 1 - dims.numInputNeurons) * cellSize.y);
					}
				}
				else
				{
					if (excitatory)
					{
						glVertex2f((synapse.fromNeuron + 0) * cellSize.x, (synapse.toNeuron + 0 - dims.numInputNeurons) * cellSize.y);
						glVertex2f((synapse.fromNeuron + 1) * cellSize.x, (synapse.toNeuron + 1 - dims.numInputNeurons) * cellSize.y);
						glVertex2f((synapse.fromNeuron + 0) * cellSize.x, (synapse.toNeuron + 1 - dims.numInputNeurons) * cellSize.y);
					}
					else
					{
						glVertex2f((synapse.fromNeuron + 1) * cellSize.x, (synapse.toNeuron + 0 - dims.numInputNeurons) * cellSize.y);
						glVertex2f((synapse.fromNeuron + 1) * cellSize.x, (synapse.toNeuron + 1 - dims.numInputNeurons) * cellSize.y);
						glVertex2f((synapse.fromNeuron + 0) * cellSize.x, (synapse.toNeuron + 0 - dims.numInputNeurons) * cellSize.y);
					}
				}

				glEnd();
			}
		}

		float activationCurr = neuralNet->GetNeuronActivation(i);
		float activationPrev = neuralNet->GetNeuronActivationPrev(i);
		
		// Draw the activation value for the current step (to the right).
		glBegin(GL_QUADS);
		if (i >= dims.numInputNeurons)
		{
			glColor3f(activationCurr, activationCurr, activationCurr);
			glVertex2f((dims.numNeurons + 1) * cellSize.x, (i + 0 - dims.numInputNeurons) * cellSize.y);
			glVertex2f((dims.numNeurons + 2) * cellSize.x, (i + 0 - dims.numInputNeurons) * cellSize.y);
			glVertex2f((dims.numNeurons + 2) * cellSize.x, (i + 1 - dims.numInputNeurons) * cellSize.y);
			glVertex2f((dims.numNeurons + 1) * cellSize.x, (i + 1 - dims.numInputNeurons) * cellSize.y);
		}

		// Draw the activation value for the previous step (below).
		Color activationColor(Vector3f(activationPrev, activationPrev, activationPrev));
		if (i < numRedNeurons)
			activationColor *= Color::RED;
		else if (i < numRedNeurons + numGreenNeurons)
			activationColor *= Color::GREEN;
		else if (i < numVisionNeurons)
			activationColor *= Color::BLUE;
		else if (i == numVisionNeurons)
			activationColor *= Color::YELLOW;
		else if (i == numVisionNeurons + 1)
			activationColor *= Color::MAGENTA;
		
		glColor4ubv(activationColor.data());
		glVertex2f((i + 0) * cellSize.x, (-2 + 0) * cellSize.y);
		glVertex2f((i + 1) * cellSize.x, (-2 + 0) * cellSize.y);
		glVertex2f((i + 1) * cellSize.x, (-2 + 1) * cellSize.y);
		glVertex2f((i + 0) * cellSize.x, (-2 + 1) * cellSize.y);
		glEnd();
	}
	
	// Calculate neuron group divisions.
	int sepCounter = dims.numInputNeurons + dims.numOutputNeurons;
	std::vector<int> groupSeps;
	groupSeps.push_back(dims.numInputNeurons);
	groupSeps.push_back(dims.numInputNeurons + dims.numOutputNeurons);
	for (int i = numInputGroups + numOutputGroups; i < agent->GetBrain()->GetNumNeuralGroups(); i++)
	{
		int numNeuronsInGroup = 
			agent->GetGenome()->GetNeuronCount(NEURON_TYPE_EXCITATORY, i) +
			agent->GetGenome()->GetNeuronCount(NEURON_TYPE_INHIBITORY, i);
		sepCounter += numNeuronsInGroup;
		groupSeps.push_back(sepCounter);
	}

	// Draw group separaters
	glBegin(GL_LINES);
	glColor3ub(255, 255, 255);
	for (int i = 0; i < (int) groupSeps.size(); i++)
	{
		int sep = groupSeps[i];
		glVertex2f(sep * cellSize.x, 0.0f);
		glVertex2f(sep * cellSize.x, numNonInputNeurons * cellSize.y);
		if (i > 0)
		{
			glVertex2f(0.0f, (sep - dims.numInputNeurons) * cellSize.y);
			glVertex2f(dims.numNeurons * cellSize.x, (sep - dims.numInputNeurons) * cellSize.y);
		}
	}
	glEnd();

	// Draw boxes around the neuron activations.
	int visNeuronStart = 0;
	g->DrawRect(visNeuronStart * cellSize.x,
				(-2) * cellSize.y,
				numRedNeurons * cellSize.x - 1.0f,
				cellSize.y, Color::RED);
	visNeuronStart += numRedNeurons;
	g->DrawRect(visNeuronStart * cellSize.x,
				(-2) * cellSize.y,
				numGreenNeurons * cellSize.x - 1.0f,
				cellSize.y, Color::GREEN);
	visNeuronStart += numGreenNeurons;
	g->DrawRect(visNeuronStart * cellSize.x,
				(-2) * cellSize.y,
				numBlueNeurons * cellSize.x,
				cellSize.y, Color::BLUE);
	visNeuronStart += numBlueNeurons;
	g->DrawRect(visNeuronStart * cellSize.x,
				(-2) * cellSize.y,
				1 * cellSize.x,
				cellSize.y, Color::YELLOW);
	visNeuronStart += 1;
	g->DrawRect(visNeuronStart * cellSize.x,
				(-2) * cellSize.y,
				1 * cellSize.x,
				cellSize.y, Color::MAGENTA);
	visNeuronStart += 1;
	g->DrawRect(visNeuronStart * cellSize.x,
				(-2) * cellSize.y,
				(dims.numNeurons - visNeuronStart) * cellSize.x,
				cellSize.y, Color::WHITE);
	
	g->DrawRect((dims.numNeurons + 1) * cellSize.x,
				0, cellSize.x, numNonInputNeurons * cellSize.x,
				Color::WHITE);

	// Draw background rectangle outline.
	g->DrawRect(Vector2f(0.0f, 0.0f),
		Vector2f((float) dims.numNeurons,
		(float) numNonInputNeurons) * cellSize,
		Color::WHITE);

	g->ResetTransform();
}

