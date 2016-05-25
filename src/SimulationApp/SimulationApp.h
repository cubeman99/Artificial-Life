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
#include <map>


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

	GraphPanel* GetGraph(const std::string& title);
	GraphPanel* CreateGraph(const std::string& title, const Color& color, float minY, float maxY, bool dynamicRange, float dynamicRangePadding = 0.1f);
	GraphPanel* CreateGraph(const std::string& title, float minY, float maxY, bool dynamicRange, float dynamicRangePadding = 0.1f);

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
	std::vector<float> m_recentFitnesses;

	// Scren layout.
	Viewport		m_panelWorld;
	Viewport		m_panelGraphs;
	Viewport		m_panelText;
	Viewport		m_panelPOV;
	Viewport		m_panelSide;
	Viewport		m_windowViewport;

	// Graphs.
	
	typedef std::map<std::string, GraphPanel*> graph_map;
	typedef std::vector<GraphPanel*> graph_list;
	
	graph_map		m_graphMap;
	graph_list		m_graphList;
	GraphPanel*		m_graph1;
	GraphPanel*		m_graph2;
	int				m_graph1Index;
	int				m_graph2Index;
	Viewport		m_graph1Viewport;
	Viewport		m_graph2Viewport;

	GraphPanel*		m_graphFitness;
	GraphPanel*		m_graphPopulation;
	GraphPanel*		m_graphEnergy;
	GraphPanel*		m_graphEnergyUsage;;	
	GraphPanel*		m_graphSize;
	GraphPanel*		m_graphStrength;
	GraphPanel*		m_graphMaxSpeed;
	GraphPanel*		m_graphFOV;
	GraphPanel*		m_graphGreenColor;
	GraphPanel*		m_graphVisionNeurons;
	GraphPanel*		m_graphNeuronGroups;
	GraphPanel*		m_graphNeurons;
	GraphPanel*		m_graphSynapses;
	GraphPanel*		m_graphMutationRate;
	GraphPanel*		m_graphCrossoverPoints;
	GraphPanel*		m_graphBirthEnergyFraction;
	GraphPanel*		m_graphLifeSpan;
	GraphPanel*		m_graphLifeExpectancy; // TODO: life expectancy.
	GraphPanel*		m_graphBehavior;

};


#endif // _SIMULATION_APP_H_