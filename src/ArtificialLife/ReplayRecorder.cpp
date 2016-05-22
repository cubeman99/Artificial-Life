#include "ReplayRecorder.h"
#include <ArtificialLife/Simulation.h>
#include <assert.h>


ReplayRecorder::ReplayRecorder(Simulation* simulation)
	: m_simulation(simulation)
	, m_isRecording(false)
	, m_fileName("")
{
}

void ReplayRecorder::BeginRecording(const std::string& fileName)
{
	assert(m_isRecording == false);

	m_fileName = fileName;
	m_file.open(m_fileName, std::ios::out | std::ios::binary);

	assert(m_file.is_open());

	// Write the header.
	m_header.magic[0] = 'c';
	m_header.magic[1] = 'm';
	m_header.magic[2] = 'a';
	m_header.magic[3] = 'i';
	m_header.worldWidth = Simulation::PARAMS.worldWidth;
	m_header.worldHeight = Simulation::PARAMS.worldHeight;
	m_header.numFrames = 0;
	m_file.write((char*) &m_header, sizeof(ReplayHeader));

	m_isRecording = true;
}

void ReplayRecorder::RecordStep()
{
	assert(m_isRecording == true);

	// Write the header for this frame.
	ReplayFrameHeader frameHeader;
	frameHeader.worldAge	= m_simulation->GetWorldAge();
	frameHeader.numAgents	= m_simulation->GetNumAgents();
	frameHeader.numFood		= m_simulation->GetNumFood();
	frameHeader.sizeInBytes	= sizeof(ReplayFrameHeader) + 
		(frameHeader.numAgents * sizeof(ReplayAgent)) +
		(frameHeader.numFood * sizeof(ReplayFood));
	m_file.write((char*) &frameHeader, sizeof(ReplayFrameHeader));
	
	// Write agent states.
	for (auto it = m_simulation->agents_begin(); it < m_simulation->agents_end(); ++it)
	{
		Agent* agent = *it;

		ReplayAgent replayAgent;
		replayAgent.id			= agent->GetID();
		replayAgent.x			= agent->GetPosition().x;
		replayAgent.y			= agent->GetPosition().y;
		replayAgent.direction	= agent->GetDirection();
		replayAgent.size		= agent->GetSize();
		replayAgent.red			= (unsigned char) (agent->GetFightAmount() * 255.0f);
		replayAgent.blue		= (unsigned char) (agent->GetMateAmount() * 255.0f);
		replayAgent.green		= (unsigned char) (agent->GetGenome()->GetGreenColoration() * 255.0f);
		m_file.write((char*) &replayAgent, sizeof(ReplayAgent));
	}

	// Write food states.
	for (auto it = m_simulation->food_begin(); it < m_simulation->food_end(); ++it)
	{
		ReplayFood replayFood;
		replayFood.x = it->GetPosition().x;
		replayFood.y = it->GetPosition().y;
		m_file.write((char*) &replayFood, sizeof(ReplayFood));
	}

	m_header.numFrames++;
}

void ReplayRecorder::StopRecording()
{
	assert(m_isRecording == true);
		
	// Re-write the header (because the number of frames has been updated).
	m_file.seekg(0, std::ios::beg);
	m_file.write((char*) &m_header, sizeof(ReplayHeader));
	
	m_file.close();

	m_isRecording = false;
}
