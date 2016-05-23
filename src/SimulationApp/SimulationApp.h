#ifndef _SIMULATION_APP_H_
#define _SIMULATION_APP_H_

#include <AppLib/Application.h>
#include <AppLib/graphics/SpriteFont.h>
#include <AppLib/graphics/Graphics.h>
#include <ArtificialLife/Camera.h>
#include <ArtificialLife/Simulation.h>
#include <ArtificialLife/SimulationParams.h>
#include <ArtificialLife/WorldRenderer.h>
#include <ArtificialLife/ReplayRecorder.h>

#include "GraphPanel.h"
#include "BrainRenderer.h"
#include <vector>


//-----------------------------------------------------------------------------

struct GenerationInfo
{
	int generationIndex;
	float bestFitness;
	float worstFitness;
	float averageFitness;
};

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
	bool			m_showInteractionRadii;
	bool			m_showGraphs;
	bool			m_showBrain;
	bool			m_followAgent;
	Vector2f		m_cursorPos;
	Agent*			m_selectedAgent;
	unsigned long	m_selectedAgentID;

	float			m_agentSelectionRadius;

	// Statistics.
	float m_totalAgentEnergy;

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