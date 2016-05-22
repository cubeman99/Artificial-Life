#ifndef _REPLAY_RECORDER_H_
#define _REPLAY_RECORDER_H_

#include <fstream>

class Simulation;


struct ReplayHeader
{
	unsigned char magic[4];

	float worldWidth;
	float worldHeight;

	int numFrames;
};

struct ReplayFrameHeader
{
	int sizeInBytes;
	int worldAge;
	int numAgents;
	int numFood;
};

struct ReplayAgent
{
	unsigned long	id;
	float			x;
	float			y;
	float			direction;
	float			size;
	unsigned char	red;
	unsigned char	green;
	unsigned char	blue;

	unsigned char	padding;
};

struct ReplayFood
{
	float x;
	float y;
};


class ReplayRecorder
{
public:
	ReplayRecorder(Simulation* simulation);

	void BeginRecording(const std::string& fileName);
	void RecordStep();
	void StopRecording();

	bool IsRecording() const { return m_isRecording; }

private:
	Simulation* m_simulation;
	std::fstream m_file;
	std::string m_fileName;
	bool m_isRecording;
	
	ReplayHeader m_header;
};


#endif // _REPLAY_RECORDER_H_