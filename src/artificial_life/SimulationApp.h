#ifndef _SIMULATION_APP_H_
#define _SIMULATION_APP_H_

#include "Simulation.h"
#include "application/Application.h"
#include "application/SpriteFont.h"
#include "application/Graphics.h"
#include "application/Renderer.h"
#include <math/Vector2f.h>
#include <math/Vector3f.h>
#include <math/Vector4f.h>
#include <math/Matrix4f.h>
#include <math/Quaternion.h>
#include "NeuronModel.h"
#include "Agent.h"
#include "Food.h"
#include "Camera.h"
#include "FittestList.h"
#include "GraphPanel.h"
#include "BrainRenderer.h"
#include "SimulationParams.h"
#include "WorldRenderer.h"
#include "ReplayRecorder.h"
#include <vector>


//-----------------------------------------------------------------------------
/*
struct GenerationInfo
{
	int generationIndex;
	float bestFitness;
	float worstFitness;
	float averageFitness;
};*/

//-----------------------------------------------------------------------------

class SimulationApp : public Application
{
public:
	typedef std::vector<Food> food_list;
	typedef std::vector<Agent*> agent_list;

public:
	SimulationApp();
	~SimulationApp();
	
protected:
	void OnInitialize() override;
	
	void ResetCamera();

	void OnUpdate(float timeDelta) override;
	void UpdateControls(float timeDelta);
	void UpdateScreenLayout();
	void UpdateStatistics();

	void OnRender() override;
	void RenderPanelWorld();
	void RenderPanelGraphs();
	void RenderPanelPOV();
	void RenderPanelText();

private:
	SpriteFont*		m_font;

	Simulation*		m_simulation;
	ReplayRecorder*	m_replayRecorder;
	BrainRenderer*	m_brainRenderer;
	
	float			m_cameraFOV;
	float			m_cameraAspect;
	ArcBallCamera	m_camera; // The main camera.
	bool			m_showFOVLines;
	bool			m_showGraphs;
	bool			m_showBrain;
	bool			m_followAgent;
	Vector2f		m_cursorPos;
	Agent*			m_selectedAgent;
	unsigned long	m_selectedAgentID;

	float			m_agentSelectionRadius;

	// Statistics.
	
	float	m_totalAgentEnergy;
	int		m_numAgentsBorn;
	int		m_numAgentsDeadOldAge;
	int		m_numAgentsDeadEnergy;
	int		m_numAgentsCreatedElite;
	int		m_numAgentsCreatedMate;
	int		m_numAgentsCreatedRandom;
	int		m_numBirthsDenied;

	std::vector<GenerationInfo> m_generationInfo;
	std::vector<float> m_recentFitnesses;

	// Scren layout.
	Viewport		m_panelWorld;
	Viewport		m_panelGraphs;
	Viewport		m_panelText;
	Viewport		m_panelPOV;
	Viewport		m_panelSide;
	Viewport		m_windowViewport;
	
	GraphPanel		m_graphFitness;
	GraphPanel		m_graphPopulation;
	GraphPanel		m_graphEnergy;

	struct Stats
	{
		float totalEnergy;
	};

	std::vector<Stats> m_simulationStats;
	std::vector<float> m_populationData;
};


#endif // _SIMULATION_APP_H_