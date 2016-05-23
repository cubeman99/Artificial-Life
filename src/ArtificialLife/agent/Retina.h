#ifndef _RETINA_H_
#define _RETINA_H_

#include <AppLib/math/Vector3f.h>
#include <AppLib/math/Matrix4f.h>
#include <ArtificialLife/brain/Nerve.h>
#include <vector>

class Retina
{
public:
	Retina();
	~Retina();

	float GetFOV() const { return m_fov; }
	void SetFOV(float fov) { m_fov = fov; }

	void Update(const float* pixels, int width);
	void UpdateNerves();

	Matrix4f GetProjection() const;

	void ConfigureChannel(int channel, Nerve* nerve);

	int GetNumChannels() const { return m_numChannels; }
	int GetNumNeurons(int channel) const;
	float GetSightValue(int channel, int neuron) const;
	float GetInterpolatedSightValue(int channel, float x) const;
		
private:
	static class Channel
	{
	public:
		friend class Retina;

		Channel();
		~Channel();

		void Configure(int channelIndex, Nerve* nerve);
		void Update(const float* pixels, int width, int numChannels);
		void UpdateNerve();

	private:
		Nerve*	m_nerve;
		int		m_channelIndex;
		int		m_numNeurons;
		float*	m_buffer;
	};

	int				m_numChannels;
	Channel*		m_channels;

	int				m_resolution;	// Width of 1-dimensional vision in pixels.
	float			m_fov;			// Field of view in radians.
};

#endif // _RETINA_H_