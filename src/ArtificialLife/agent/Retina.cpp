#include "Retina.h"
#include <AppLib/math/MathLib.h>


//-----------------------------------------------------------------------------
// Retina
//-----------------------------------------------------------------------------

Retina::Retina()
	: m_resolution(64)
	, m_numChannels(3)
	, m_fov(0.8f)
{
	m_channels = new Channel[m_numChannels];
}

Retina::~Retina()
{
	delete [] m_channels; m_channels = NULL;
}

Matrix4f Retina::GetProjection() const
{
	return Matrix4f::CreatePerspectiveXY(m_fov, 0.01f, 0.1f, 1000.0f);
}

int Retina::GetNumNeurons(int channel) const
{
	return m_channels[channel].m_numNeurons;
}

float Retina::GetSightValue(int channel, int neuron) const
{
	return m_channels[channel].m_buffer[neuron];
}

float Retina::GetInterpolatedSightValue(int channel, float x) const
{
	int numNeurons = m_channels[channel].m_numNeurons;
	
	if (numNeurons == 0)
	{
		return 0.0f;
	}
	else if (numNeurons == 1)
	{
		return GetSightValue(channel, 0);
	}
	else
	{
		x = Math::Clamp(x, 0.0f, 1.0f);

		float neuronIndex = (x * numNeurons) - 0.5f;
		int neuron0 = Math::Max((int) neuronIndex + 0, 0);
		int neuron1 = Math::Min((int) neuronIndex + 1, numNeurons - 1);
		float t = neuronIndex - (float) neuron0;

		return Math::Lerp(GetSightValue(channel, neuron0),
						  GetSightValue(channel, neuron1),
						  t);
	}
}

void Retina::ConfigureChannel(int channel, Nerve* nerve)
{
	m_channels[channel].Configure(channel, nerve);
}

void Retina::Update(const float* pixels, int width)
{
	for (int i = 0; i < m_numChannels; i++)
		m_channels[i].Update(pixels, width, m_numChannels);
}

void Retina::UpdateNerves()
{
	for (int i = 0; i < m_numChannels; i++)
		m_channels[i].UpdateNerve();
}



//-----------------------------------------------------------------------------
// Channel
//-----------------------------------------------------------------------------

Retina::Channel::Channel()
	: m_channelIndex(-1)
	, m_numNeurons(0)
	, m_buffer(NULL)
	, m_nerve(NULL)
{
}

Retina::Channel::~Channel()
{
	delete [] m_buffer;
	m_buffer = NULL;
}

void Retina::Channel::Configure(int channelIndex, Nerve* nerve)
{
	if (m_buffer)
		delete [] m_buffer;

	m_nerve			= nerve;
	m_channelIndex	= channelIndex;
	m_numNeurons	= nerve->GetNumNeurons();
	m_buffer		= new float[m_numNeurons];
}

void Retina::Channel::Update(const float* pixels, int width, int numChannels)
{			
	int neuronIndex, nextNeuronIndex;
	float remainder;

	float numNeuronsPerPixel = (float) m_numNeurons / (float) width;
	float numPixelsPerNeuron = (float) width / (float) m_numNeurons;
	const float* pixel = pixels + m_channelIndex;
	
	// First, zero the neurons.
	for (int i = 0; i < m_numNeurons; i++)
		m_buffer[i] = 0.0f;
	
	// Then, add the average pixel colors for each neuron.
	for (int i = 0; i < width; i++)
	{
		neuronIndex = (int) (i * numNeuronsPerPixel);
		nextNeuronIndex = (int) (Math::Min(i + 1, width - 1) * numNeuronsPerPixel);

		if (nextNeuronIndex > neuronIndex)
		{
			// A division between two neurons happens at this pixel!
			// Divide this pixel's color accordingly between the two neurons.

			// Neurons: [  N0  ][  N1  ][  N2  ] n = 3
			// Pixels:  [0][1][2][3][4][5][6][7] w = 8
			//                 ^^
			// division between N0 and N1 at pixel 2.

			remainder = (numPixelsPerNeuron * nextNeuronIndex) - i;
			m_buffer[neuronIndex]     += (*pixel) * remainder * numNeuronsPerPixel;
			m_buffer[nextNeuronIndex] += (*pixel) * (1.0f - remainder) * numNeuronsPerPixel;
		}
		else
		{
			m_buffer[neuronIndex] += (*pixel) * numNeuronsPerPixel;
		}

		pixel += numChannels;
	}
}

void Retina::Channel::UpdateNerve()
{
	for (int i = 0; i < m_numNeurons; i++)
		m_nerve->Set(i, m_buffer[i]);
}

