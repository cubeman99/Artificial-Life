#ifndef _SIMULATION_H_
#define _SIMULATION_H_

#include "application/Application.h"
#include "application/SpriteFont.h"
#include "application/Graphics.h"
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

class Simulation : public Application
{
public:
	Simulation();
	~Simulation();
	
	unsigned long GetNewAgentID()
	{
		return m_agentCounter++;
	}

protected:
	void OnInitialize() override;
	void LoadModels();
	
	void ResetCamera();

	void OnUpdate(float timeDelta) override;
	void UpdateWorld(float timeDelta);
	void RenderAgentVision(Agent* agent);
	void UpdateScreenLayout();

	void OnRender() override;
	void RenderWorld(ICamera* camera, Agent* agent = NULL);
	void RenderPanelWorld();
	void RenderPanelGraphs();
	void RenderPanelPOV();
	void RenderPanelText();

	Agent* Mate(Agent* mommy, Agent* daddy);
	void Kill(Agent* agent);
	void PickParentsUsingTournament(int numInPool, int* iParent, int* jParent);
	
	// UNUSED:
	void NextGeneration();
	Agent* AgentRoulette();

private:

	unsigned long m_agentCounter;

	// Resources.
	SpriteFont* m_font;
	std::vector<Vector3f> m_agentVertices;
	std::vector<Vector3f> m_foodVertices;

	Vector2f m_worldDimensions;

	std::vector<Agent*> m_agents;
	std::vector<Food> m_food;
	
	int m_generationTickCounter;
	int m_generationTickDuration;
	int m_generation;
	int m_worldAge;
	int m_numAgentsBorn;
	int m_numAgentsDeadOldAge;
	int m_numAgentsDeadEnergy;
	int m_numAgentsCreatedElite;
	int m_numAgentsCreatedMate;
	int m_numAgentsCreatedRandom;
	int m_numBirthsDenied;

	int m_numAgents;
	int m_numElites;
	int m_numEliteCopies;
	
	float m_cameraFOV;
	float m_cameraAspect;
	ArcBallCamera m_camera; // The main camera.

	Vector2f m_cursorPos;

	float m_agentSelectionRadius;

	std::vector<GenerationInfo> m_generationInfo;
	FittestList* m_fittestList;
	std::vector<float> m_recentFitnesses;
	
	bool m_showFOVLines;
	bool m_showGraphs;
	bool m_showBrain;
	bool m_followAgent;
	Agent* m_selectedAgent;

	struct Stats
	{
		float totalEnergy;
	};

	// Scren layout.
	Viewport m_panelWorld;
	Viewport m_panelGraphs;
	Viewport m_panelText;
	Viewport m_panelPOV;
	Viewport m_panelSide;
	Viewport m_windowViewport;

	BrainRenderer	m_brainRenderer;
	GraphPanel		m_graphFitness;
	GraphPanel		m_graphPopulation;
	GraphPanel		m_graphEnergy;

	std::vector<Stats> m_simulationStats;
	std::vector<float> m_populationData;


	
public:
	static SimulationParams PARAMS;
};


#endif // _SIMULATION_H_