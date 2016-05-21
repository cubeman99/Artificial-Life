#include "Retina.h"
#include "math/MathLib.h"


//-----------------------------------------------------------------------------
// Retina
//-----------------------------------------------------------------------------

Retina::Retina()
	: m_resolution(64)
	, m_numChannels(3)
	, m_fov(0.8f)
{
	m_channels	= new Channel[m_numChannels];
	m_buffer	= new unsigned char[m_resolution * m_numChannels];

	for (int i = 0; i < m_numChannels; i++)
		m_channels[i].Init(i, 3);
}

Retina::~Retina()
{
	delete [] m_buffer; m_buffer = NULL;
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

void Retina::SetNumNeurons(int channel, int numNeurons)
{
	m_channels[channel].Init(channel, numNeurons);
}

void Retina::Update(const float* pixels, int width)
{
	for (int i = 0; i < m_numChannels; i++)
		m_channels[i].Update(pixels, width, m_numChannels);
}



//-----------------------------------------------------------------------------
// Channel
//-----------------------------------------------------------------------------

Retina::Channel::Channel()
	: m_channelIndex(-1)
	, m_numNeurons(0)
	, m_buffer(NULL)
{
}

Retina::Channel::~Channel()
{
	delete [] m_buffer;
	m_buffer = NULL;
}

void Retina::Channel::Init(int channelIndex, int numNeurons)
{
	if (m_buffer)
		delete [] m_buffer;

	m_channelIndex	= channelIndex;
	m_numNeurons	= numNeurons;
	m_buffer		= new float[m_numNeurons];
}

void Retina::Channel::Update(const float* pixels, int width, int numChannels)
{
	for (int i = 0; i < m_numNeurons; i++)
		m_buffer[i] = 0.0f;
			
	float numNeuronsPerPixel = (float) m_numNeurons / (float) width;
	const float* pixel = pixels + m_channelIndex;
	int neuronIndex;

	for (int i = 0; i < width; i++)
	{
		neuronIndex = (int) (i * numNeuronsPerPixel);
		m_buffer[neuronIndex] += (*pixel) * numNeuronsPerPixel;
		pixel += numChannels;
	}
}

