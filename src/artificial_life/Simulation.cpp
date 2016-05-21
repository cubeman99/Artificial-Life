#include "Simulation.h"
#include "application/Graphics.h"
#include "math/MathLib.h"
#include "math/Vector4f.h"
#include "util/Timing.h" 
#include "Brain.h"
#include <algorithm> // for std::sort


SimulationParams Simulation::PARAMS;


Simulation::Simulation()
	: m_fittestList(NULL)
{
}

Simulation::~Simulation()
{
	delete m_font; m_font = NULL;

	// Delete all agents.
	for (unsigned int i = 0; i < m_agents.size(); i++)
		delete m_agents[i];
	m_agents.clear();

	delete m_fittestList; m_fittestList = NULL;
}



void Simulation::OnInitialize()
{
	Random::SeedTime();
	
	//-----------------------------------------------------------------------------
	// Load resources.

	m_font = new SpriteFont("../assets/font_console.png", 16, 8, 12, 0);
	LoadModels();
	
	//-----------------------------------------------------------------------------
	// Configure graphs.

	Color colorBestFitness		= Color::GREEN;
	Color colorAverageFitness	= Color::YELLOW;
	Color colorWorstFitness		= Color::RED;
	
	m_graphEnergy.SetTitle("energy");
	m_graphEnergy.SetFont(m_font);
	m_graphFitness.SetViewBounds(0, 1000, 0, 30);
	m_graphEnergy.AddGraph(Color::YELLOW, 0, 1);

	m_graphPopulation.SetTitle("population");
	m_graphPopulation.SetFont(m_font);
	m_graphFitness.SetViewBounds(0, 1000, 0, 30);
	m_graphPopulation.AddGraph(Color::CYAN, 0, 1);
	
	m_graphFitness.SetTitle("fitness");
	m_graphFitness.SetFont(m_font);
	m_graphFitness.SetViewBounds(0, 10, 0, 25);
	m_graphFitness.AddGraph(Color::GREEN, 1, 4); // best
	m_graphFitness.AddGraph(Color::RED, 2, 4); // worst
	m_graphFitness.AddGraph(Color::YELLOW, 3, 4); // average

	//-----------------------------------------------------------------------------
	// Reset simulation variables.

	m_agentSelectionRadius = 20;

	m_numAgents				= 30;
	m_numElites				= 4;
	m_numEliteCopies		= 1;
	
	m_generationTickCounter		= 0;
	m_generationTickDuration	= 2000;

	m_generation				= 0;
	m_worldAge					= 0;
	m_agentCounter				= 0;
	m_numAgentsBorn				= 0;
	m_numAgentsDeadOldAge		= 0;
	m_numAgentsDeadEnergy		= 0;
	m_numAgentsCreatedElite		= 0;
	m_numAgentsCreatedMate		= 0;
	m_numAgentsCreatedRandom	= 0;
	m_numBirthsDenied			= 0;

	m_showFOVLines			= false;
	m_showGraphs			= false;
	m_showBrain				= false;
	m_followAgent			= false;
	m_selectedAgent			= NULL;

	m_totalAgentEnergy = 0.0f;

	m_cameraFOV				= 80.0f * Math::DEG_TO_RAD;

	//-----------------------------------------------------------------------------
	// Simulation globals.
	
	PARAMS.worldWidth				= 1300;
	PARAMS.worldHeight				= 1300;
	PARAMS.boundaryType				= BoundaryType::BOUNDARY_TYPE_WRAP;

	PARAMS.minAgents				= 45;//35;
	PARAMS.maxAgents				= 150;//120;
	PARAMS.minFood					= 220;
	PARAMS.maxFood					= 300;
	PARAMS.initialFoodCount			= 220;
	PARAMS.initialNumAgents			= 45;
		
	//-----------------------------------------------------------------------------
	// Energy and fitness parameters.
    
	PARAMS.numFittest				= 10;
	PARAMS.pairFrequency			= 100;
	PARAMS.eliteFrequency			= 2;

	// Parameters for measuring an agent's fitness.
	PARAMS.eatFitnessParam			= 1.0f;
	PARAMS.mateFitnessParam			= 10.0f;
	PARAMS.moveFitnessParam			= 1.0f / 800.0f;
	PARAMS.energyFitnessParam		= 2.0f;
	PARAMS.ageFitnessParam			= 0.03f;
	
	// Energy costs.
	PARAMS.energyCostEat			= 0.0f;
	PARAMS.energyCostMate			= 0.003f;
	PARAMS.energyCostFight			= 0.0f;
	PARAMS.energyCostMove			= 0.0f;//0.002f;
	PARAMS.energyCostTurn			= 0.0f; //0.002f;
	PARAMS.energyCostNeuron			= 0.0f; // TODO: find a value for this.
	PARAMS.energyCostSynapse		= 0.0f; // TODO: find a value for this.
	PARAMS.energyCostExist			= 0.001f;

	//float maxsynapse2energy; // (amount if all synapses usable)
	//float maxneuron2energy;

	//-----------------------------------------------------------------------------
	// Agent configuration.
	
	PARAMS.mateWait					= 120;
	PARAMS.initialMateWait			= 120;
	PARAMS.retinaResolution			= 16;
	PARAMS.retinaVerticalFOV		= 0.01f;

	//-----------------------------------------------------------------------------
	// Agent gene ranges.

	PARAMS.minFOV					= 20.0f * Math::DEG_TO_RAD;
	PARAMS.maxFOV					= 130.0f * Math::DEG_TO_RAD;
	PARAMS.minStrength				= 0.0f;
	PARAMS.maxStrength				= 1.0f;
	PARAMS.minSize					= 0.7f;
	PARAMS.maxSize					= 1.6f;
	PARAMS.minMaxSpeed				= 1.0f;
	PARAMS.maxMaxSpeed				= 2.5f;
	PARAMS.minMutationRate			= 0.01f;
	PARAMS.maxMutationRate			= 0.1f;
	PARAMS.minNumCrossoverPoints	= 2;
	PARAMS.maxNumCrossoverPoints	= 6; // supposed to be 8
	PARAMS.minLifeSpan				= 1500;
	PARAMS.maxLifeSpan				= 2800;
	PARAMS.minBirthEnergyFraction	= 0.1f;
	PARAMS.maxBirthEnergyFraction	= 0.7f;
	PARAMS.minVisNeuronsPerGroup	= 1;
	PARAMS.maxVisNeuronsPerGroup	= 16;
	PARAMS.minInternalNeuralGroups	= 1;
	PARAMS.maxInternalNeuralGroups	= 5;

	PARAMS.minENeuronsPerGroup		= 1;
	PARAMS.maxENeuronsPerGroup		= 6;
	PARAMS.minINeuronsPerGroup		= 1;
	PARAMS.maxINeuronsPerGroup		= 6; // supposed to be 16

	PARAMS.minConnectionDensity		= 0.0f;
	PARAMS.maxConnectionDensity		= 1.0f;
	PARAMS.minTopologicalDistortion	= 0.0f;
	PARAMS.maxTopologicalDistortion	= 1.0f;
	PARAMS.minSynapseLearningRate	= 0.0f;
	PARAMS.maxSynapseLearningRate	= 0.1f;
	
	//-----------------------------------------------------------------------------
	// Brain configuration.

	PARAMS.numInputNeurGroups		= 5; // red, green, blue, energy, random
	PARAMS.numOutputNeurGroups		= 5; // speed, turn, mate, fight, eat (MISSING focus and light).
	PARAMS.numPrebirthCycles		= 10;
	PARAMS.maxBias					= 1.0f;
	PARAMS.minBiasLearningRate		= 0.0f; // unused
	PARAMS.maxBiasLearningRate		= 0.1f; // unused
	PARAMS.logisticSlope			= 1.0f;
	PARAMS.maxWeight				= 1.0f;
	PARAMS.initMaxWeight			= 0.5f;
	PARAMS.decayRate				= 0.99f;
	
	//-----------------------------------------------------------------------------
	
	m_worldDimensions.x = Simulation::PARAMS.worldWidth;
	m_worldDimensions.y = Simulation::PARAMS.worldHeight;
	m_fittestList = new FittestList(Simulation::PARAMS.numFittest);
	
	m_graphPopulation.SetViewBounds(0, 120,
		(float) (Simulation::PARAMS.minAgents / 2),
		(float) ((int) (Simulation::PARAMS.maxAgents * 1.2f)));

	//-----------------------------------------------------------------------------

	// Setup OpenGL state.
	glDepthMask(true);
    glEnable(GL_DEPTH_CLAMP);
	
	UpdateScreenLayout();
	ResetCamera();
	
	//-----------------------------------------------------------------------------
	// Initialize world.

	// Create initial food.
	m_food.resize(Simulation::PARAMS.initialFoodCount);
	for (int i = 0; i < Simulation::PARAMS.initialFoodCount; i++)
	{
		m_food[i].SetPosition(Vector2f(
			Random::NextFloat() * m_worldDimensions.x,
			Random::NextFloat() * m_worldDimensions.y));
	}
	
	// Create initial agents with random genomes.
	for (int i = 0; i < Simulation::PARAMS.initialNumAgents; i++)
	{
		Agent* agent = new Agent(this);
		agent->GetGenome()->Randomize();
		agent->Grow();
		agent->SetPosition(Vector2f(
			Random::NextFloat() * m_worldDimensions.x,
			Random::NextFloat() * m_worldDimensions.y));
		m_agents.push_back(agent);
	}
}

void Simulation::LoadModels()
{
	// Create agent model.
	{
		float ah = 5.0f; // agent height.
		Vector3f v1( 10.0f,  0.0f, 0.0f);
		Vector3f v2(-10.0f, -9.0f, 0.0f);
		Vector3f v3(-10.0f,  9.0f, 0.0f);
		Vector3f v4( 10.0f,  0.0f, ah);
		Vector3f v5(-10.0f, -9.0f, ah);
		Vector3f v6(-10.0f,  9.0f, ah);

		m_agentVertices.push_back(v1); // Bottom
		m_agentVertices.push_back(v2);
		m_agentVertices.push_back(v3);
		m_agentVertices.push_back(v4); // Top
		m_agentVertices.push_back(v6);
		m_agentVertices.push_back(v5);
		m_agentVertices.push_back(v1); // Left
		m_agentVertices.push_back(v4);
		m_agentVertices.push_back(v5);
		m_agentVertices.push_back(v1);
		m_agentVertices.push_back(v5);
		m_agentVertices.push_back(v2);
		m_agentVertices.push_back(v3); // Right
		m_agentVertices.push_back(v6);
		m_agentVertices.push_back(v4);
		m_agentVertices.push_back(v3);
		m_agentVertices.push_back(v4);
		m_agentVertices.push_back(v1);
		m_agentVertices.push_back(v2); // Back
		m_agentVertices.push_back(v5);
		m_agentVertices.push_back(v6);
		m_agentVertices.push_back(v2);
		m_agentVertices.push_back(v6);
		m_agentVertices.push_back(v3);
	}
	
	// Create food model.
	{
		float fw = 4.0f; // food width.
		float fh = 5.0f; // food height.

		Vector3f v1(-fw, -fw, 0.0f);
		Vector3f v2(-fw,  fw, 0.0f);
		Vector3f v3( fw,  fw, 0.0f);
		Vector3f v4( fw, -fw, 0.0f);
		Vector3f v5(-fw, -fw, fh);
		Vector3f v6(-fw,  fw, fh);
		Vector3f v7( fw,  fw, fh);
		Vector3f v8( fw, -fw, fh);

		m_foodVertices.push_back(v2); // Bottom
		m_foodVertices.push_back(v1);
		m_foodVertices.push_back(v4);
		m_foodVertices.push_back(v3);

		m_foodVertices.push_back(v5); // Top
		m_foodVertices.push_back(v6);
		m_foodVertices.push_back(v7);
		m_foodVertices.push_back(v8);

		m_foodVertices.push_back(v1); // Front
		m_foodVertices.push_back(v5);
		m_foodVertices.push_back(v8);
		m_foodVertices.push_back(v4);
	
		m_foodVertices.push_back(v3); // Back
		m_foodVertices.push_back(v7);
		m_foodVertices.push_back(v6);
		m_foodVertices.push_back(v2);

		m_foodVertices.push_back(v2); // Left
		m_foodVertices.push_back(v6);
		m_foodVertices.push_back(v5);
		m_foodVertices.push_back(v1);

		m_foodVertices.push_back(v4); // Right
		m_foodVertices.push_back(v8);
		m_foodVertices.push_back(v7);
		m_foodVertices.push_back(v3);
	}
}

void Simulation::ResetCamera()
{
	m_camera.position.x = m_worldDimensions.x * 0.5f;
	m_camera.position.y = m_worldDimensions.y * 0.5f;
	m_camera.position.z = 0.0f;
	m_camera.rotation.SetIdentity();
	
	m_cameraAspect		= m_panelWorld.GetAspectRatio();
	m_camera.projection	= Matrix4f::CreatePerspective(m_cameraFOV, m_cameraAspect, 4.0f, 3000.0f);

	float z = (float) (Simulation::PARAMS.worldHeight * 1.05f) / (2.0f * Math::Tan(m_cameraFOV * 0.5f));
	m_camera.distance = z;
}


//-----------------------------------------------------------------------------
// Simulation Update
//-----------------------------------------------------------------------------

void Simulation::OnUpdate(float timeDelta)
{
	UpdateWorld();
	UpdateControls(timeDelta);
	UpdateScreenLayout();
	UpdateStatistics();
}

void Simulation::UpdateControls(float timeDelta)
{
	Keyboard* keyboard = GetKeyboard();
	Mouse* mouse = GetMouse();

	//-----------------------------------------------------------------------------
	// Simulation controls.
		
	// Escape: Exit the program.
	if (keyboard->IsKeyPressed(Keys::ESCAPE))
	{
		Quit();
		return;
	}
	
	// Enter: Next generation.
	if (keyboard->IsKeyPressed(Keys::ENTER))
	{
		NextGeneration();
		m_generationTickCounter = 0;
	}

	// G: Show/hide graphs.
	if (keyboard->IsKeyPressed(Keys::G))
		m_showGraphs = !m_showGraphs;
	
	// B: Show/hide brain connectivity matrix.
	if (keyboard->IsKeyPressed(Keys::B))
		m_showBrain = !m_showBrain;
	
	// B: Show/hide agent FOV/vision lines.
	if (keyboard->IsKeyPressed(Keys::O))
		m_showFOVLines = !m_showFOVLines;
	
	// G: Follow selected agent.
	if (keyboard->IsKeyPressed(Keys::F))
		m_followAgent = !m_followAgent;

	// Ctrl + D: Deselect.
	if (keyboard->IsKeyPressed(Keys::D) && keyboard->IsKeyDown(Keys::LCONTROL))
		m_selectedAgent = NULL;

	//-----------------------------------------------------------------------------
	// Camera controls.

	Vector3f cameraForwardMove = m_camera.rotation.GetUp() + m_camera.rotation.GetForward();
	cameraForwardMove.SetXY(cameraForwardMove.GetXY().Normalize());
	cameraForwardMove.z = 0.0f;
	Vector3f cameraRightMove = m_camera.rotation.GetRight();

	// TODO: magic numbers.
	float minCamMoveSpeed	= 100.0f;
	float maxCamMoveSpeed	= 2500.0f;
	float minCamDist		= 20.0f;
	float maxCamDist		= 1500.0f;
	float camRotateSpeed	= 2.0f;
	float camRotateRadians	= 0.006f;

	float camZoomPercent	= 1.0f - ((m_camera.distance - minCamDist) / (maxCamDist - minCamDist));
	float camMoveSpeed		= Math::Lerp(maxCamMoveSpeed, minCamMoveSpeed, camZoomPercent);
	float camMousePanAmount	= camMoveSpeed / 1000.0f;
	
	// WASD or Middle Mouse: Move camera.
	if (keyboard->IsKeyDown(Keys::W))
		m_camera.position += cameraForwardMove * camMoveSpeed * timeDelta;
	if (keyboard->IsKeyDown(Keys::S))
		m_camera.position -= cameraForwardMove * camMoveSpeed * timeDelta;
	if (keyboard->IsKeyDown(Keys::D))
		m_camera.position += cameraRightMove * camMoveSpeed * timeDelta;
	if (keyboard->IsKeyDown(Keys::A))
		m_camera.position -= cameraRightMove * camMoveSpeed * timeDelta;
	if (mouse->IsButtonDown(MouseButtons::MIDDLE))
	{
		m_camera.position -= cameraRightMove   * camMousePanAmount * (float) mouse->GetDeltaX();
		m_camera.position += cameraForwardMove * camMousePanAmount * (float) mouse->GetDeltaY();
	}

	// Arrow Keys or Left Mouse: Rotate camera.
	if (keyboard->IsKeyDown(Keys::NUMPAD_6)) // right
		m_camera.rotation.Rotate(Vector3f::UNITZ, camRotateSpeed * timeDelta);
	if (keyboard->IsKeyDown(Keys::NUMPAD_4)) // left
		m_camera.rotation.Rotate(Vector3f::UNITZ, -camRotateSpeed * timeDelta);
	if (keyboard->IsKeyDown(Keys::NUMPAD_8)) // up
		m_camera.rotation.Rotate(m_camera.rotation.GetRight(), -camRotateSpeed * timeDelta);
	if (keyboard->IsKeyDown(Keys::NUMPAD_5)) // down
		m_camera.rotation.Rotate(m_camera.rotation.GetRight(), camRotateSpeed * timeDelta);
	if (mouse->IsButtonDown(MouseButtons::RIGHT))
	{
		m_camera.rotation.Rotate(Vector3f::UNITZ, camRotateRadians * mouse->GetDeltaX());
		m_camera.rotation.Rotate(m_camera.rotation.GetRight(), camRotateRadians * mouse->GetDeltaY());
	}

	// Scroll Wheel or Right Mouse: Zoom in/out.
	if (mouse->GetDeltaZ() != 0)
		m_camera.distance = m_camera.distance * powf(0.9f, (float) mouse->GetDeltaZ());
	if (keyboard->IsKeyDown(Keys::NUMPAD_PLUS))
		m_camera.distance = m_camera.distance * powf(0.1f, timeDelta);
	if (keyboard->IsKeyDown(Keys::NUMPAD_MINUS))
		m_camera.distance = m_camera.distance * powf(0.1f, -timeDelta);
	m_camera.distance = Math::Clamp(m_camera.distance, minCamDist, maxCamDist);

	// Home: Reset the camera.
	if (keyboard->IsKeyPressed(Keys::HOME))
		ResetCamera();
	
	// Update agent camera following.
	if (m_followAgent && m_selectedAgent != NULL)
	{
		m_camera.position.SetXY(m_selectedAgent->GetPosition());
	}
	
	//-----------------------------------------------------------------------------
	// Calculate cursor world position on the ground plane.

	Vector2f winSize((float) m_panelWorld.width, (float) m_panelWorld.height);
	Vector2f mouseOff((float) GetMouse()->GetX() - m_panelWorld.x, (float) GetMouse()->GetY() - m_panelWorld.y);
	mouseOff -= winSize * 0.5f;
	mouseOff /= winSize * 0.5f;
	
	float	 planeDist		= 0.0f;
	Vector3f planeNormal	= Vector3f::UNITZ;
	Vector3f rayPos			= m_camera.GetViewPosition();
	Vector3f rayNormal		= m_camera.rotation.GetForward();
	
	// Screen point to world ray.
	float zdist = 1.0f / Math::Tan(m_cameraFOV * 0.5f);
	rayNormal.z = -zdist;
	rayNormal.x = mouseOff.x * m_cameraAspect;
	rayNormal.y = -mouseOff.y;
	rayNormal.Normalize();
	rayNormal.Rotate(m_camera.rotation);

	// Raycast onto the ground plane.
	Vector3f cameraLookPoint = Vector3f::ZERO;
    float denom = planeNormal.Dot(rayNormal);
    if (denom < -0.0001f)
	{
        float distance = Vector3f::Dot((planeNormal * planeDist) - rayPos, planeNormal) / denom; 
		cameraLookPoint = rayPos + (rayNormal * distance);
    }

	m_cursorPos = cameraLookPoint.GetXY();
	
	//-------------------------------------------------------
	// Left click: select agents.

	if (mouse->IsButtonPressed(MouseButtons::LEFT))
	{
		m_selectedAgent = NULL;
		float nearestAgentDist = 0.0f;
		Agent* nearestAgent = NULL;

		// Find the agent closest to the mouse cursor.
		for (unsigned int i = 0; i < m_agents.size(); i++)
		{
			float dist = Vector2f::Dist(m_cursorPos, m_agents[i]->GetPosition());
			if (dist < nearestAgentDist || nearestAgent == NULL)
			{
				nearestAgent = m_agents[i];
				nearestAgentDist = dist;
			}
		}

		if (nearestAgent != NULL && nearestAgentDist < m_agentSelectionRadius)
			m_selectedAgent = nearestAgent;
	}
}

void Simulation::UpdateScreenLayout()
{
	// This stuff updates things that are dependent on the window dimensions.

	//-----------------------------------------------------------------------------
	// Layout UI panels.

	int screenWidth		= GetWindow()->GetWidth();
	int screenHeight	= GetWindow()->GetHeight();
	int sideBarWidth	= 300;
	int graphBarHeight	= 100;

	m_panelWorld.x		= 0;
	m_panelWorld.y		= graphBarHeight;
	m_panelWorld.width	= screenWidth - sideBarWidth;
	m_panelWorld.height	= screenHeight - graphBarHeight;

	m_panelSide.x		= screenWidth - sideBarWidth;
	m_panelSide.y		= 0;
	m_panelSide.width	= sideBarWidth;
	m_panelSide.height	= screenHeight;

	m_panelText.x		= screenWidth - sideBarWidth;
	m_panelText.y		= 0;
	m_panelText.width	= sideBarWidth;
	m_panelText.height	= screenHeight - sideBarWidth;

	m_panelPOV.x		= screenWidth - sideBarWidth;
	m_panelPOV.y		= screenHeight - sideBarWidth;
	m_panelPOV.width	= sideBarWidth;
	m_panelPOV.height	= sideBarWidth;

	m_panelGraphs.x		= 0;
	m_panelGraphs.y		= 0;
	m_panelGraphs.width	= screenWidth - sideBarWidth;
	m_panelGraphs.height = graphBarHeight;

	m_windowViewport.x		= 0;
	m_windowViewport.y		= 0;
	m_windowViewport.width	= screenWidth;
	m_windowViewport.height	= screenHeight;
		
	Viewport graphViewport1(
		m_panelGraphs.x,
		m_panelGraphs.y,
		m_panelGraphs.width / 2,
		m_panelGraphs.height);
	Viewport graphViewport2(
		m_panelGraphs.x + (m_panelGraphs.width / 2),
		m_panelGraphs.y,
		m_panelGraphs.width / 2,
		m_panelGraphs.height);
	graphViewport1.Inset(8);
	graphViewport2.Inset(8);

	m_graphEnergy.SetViewport(graphViewport1);
	m_graphPopulation.SetViewport(graphViewport2);
	m_graphFitness.SetViewport(graphViewport2);

	// Update the camera's projection.
	m_cameraAspect		= m_panelWorld.GetAspectRatio();
	m_camera.projection	= Matrix4f::CreatePerspective(m_cameraFOV, m_cameraAspect, 4.0f, 3000.0f);
}

void Simulation::UpdateStatistics()
{
	// Update world statistics.
	if (m_worldAge % 60 == 0)
	{
		float totalEnergy = 0.0f;
		for (unsigned int i = 0; i < m_agents.size(); i++)
			totalEnergy += m_agents[i]->GetEnergy();
		
		Stats stats;
		stats.totalEnergy = totalEnergy;
		m_simulationStats.push_back(stats);
		m_populationData.push_back((float) m_agents.size());
		m_graphEnergy.SetData((float*) &m_simulationStats[0], (int) m_simulationStats.size());
		m_graphPopulation.SetData((float*) &m_populationData[0], (int) m_populationData.size());

		float totalfit = 0.0f;
		float avgFit = 0.0f;
		float worstFit = 0.0f;
		float bestFit = 0.0f;
		if (m_recentFitnesses.size() > 0)
		{
			for (unsigned int i = 0; i < m_recentFitnesses.size(); i++)
			{
				totalfit += m_recentFitnesses[i];
				if (m_recentFitnesses[i] < worstFit || i == 0)
					worstFit = m_recentFitnesses[i];
				if (m_recentFitnesses[i] > bestFit || i == 0)
					bestFit = m_recentFitnesses[i];
			}
			avgFit = totalfit / (float) m_recentFitnesses.size();
		}
		GenerationInfo genInfo;
		genInfo.averageFitness = avgFit;
		genInfo.worstFitness = worstFit;
		genInfo.bestFitness = bestFit;
		genInfo.generationIndex = (int) m_generationInfo.size();
		m_generationInfo.push_back(genInfo);
	}
	
	m_graphFitness.SetData((float*) &m_generationInfo[0], (int) m_generationInfo.size() * 4);
}


//-----------------------------------------------------------------------------
// Update World
//-----------------------------------------------------------------------------

void Simulation::UpdateWorld()
{
	m_worldAge++;

	UpdateAgents();
	UpdateSteadyStateGA();
	UpdateFood();
}

void Simulation::UpdateAgents()
{
	m_totalAgentEnergy = 0.0f;

	// Update all agents.
	for (unsigned int i = 0; i < m_agents.size(); i++)
	{
		Agent* agent = m_agents[i];
		Vector2f agentPos = agent->GetPosition();
		
		m_totalAgentEnergy += agent->GetEnergy();

		// Find food near the agent.
		for (unsigned int j = 0; j < m_food.size(); j++)
		{
			float distToFood = Vector2f::Dist(agentPos, m_food[j].GetPosition());
			if (distToFood < 15.0f)
			{
				m_food[j].SetPosition(Vector2f(
					Random::NextFloat() * m_worldDimensions.x,
					Random::NextFloat() * m_worldDimensions.y));
				agent->AddEnergy(m_food[j].GetEnergyValue());
				agent->OnEat();
				m_food.erase(m_food.begin() + j);
				j--;
			}
		}
		
		// Update the agent.
		agent->Update();
		RenderAgentVision(agent);
		
		// Kill the agent if its energy drops below zero.
		if (agent->GetEnergy() <= 0.0f || agent->GetAge() >= agent->GetLifeSpan())
		{
			if (agent->GetAge() >= agent->GetLifeSpan())
				m_numAgentsDeadOldAge++;
			else if (agent->GetEnergy() <= 0.0f)
				m_numAgentsDeadEnergy++;

			Kill(agent);
			m_agents.erase(m_agents.begin() + i);
			i--;
		}
	}
	
	float mateThreshhold = 0.6f;

	// Mate agents.
	int numAgents = (int) m_agents.size();
	for (int i = 0; i < numAgents; i++)
	{
		Agent* mommy = m_agents[i];

		for (int j = 0; j < numAgents; j++)
		{
			if (i == j)
				continue;

			Agent* daddy = m_agents[j];
			float dist = Vector2f::Dist(mommy->GetPosition(), daddy->GetPosition());
				
			if (dist < 60 &&
				mommy->CanMate() &&
				daddy->CanMate() &&
				mommy->GetMateAmount() > mateThreshhold &&
				daddy->GetMateAmount() > mateThreshhold)// &&
				//Random::NextFloat() < (mommy->GetMateAmount() * daddy->GetMateAmount()) * 0.001f)
			{
				Agent* child = Mate(mommy, daddy);
				if (child != NULL)
				{
					child->SetPosition((mommy->GetPosition() + daddy->GetPosition()) * 0.5f);
					m_agents.push_back(child);
				}
				break;
			}
		}
	}
}

void Simulation::UpdateFood()
{
	// TODO: Better food.

	// Spawn food at a constant rate.
	if ((int) m_food.size() < Simulation::PARAMS.minFood && m_worldAge % 4 == 0)
	{
		Food food;
		food.SetPosition(Vector2f(
			Random::NextFloat() * m_worldDimensions.x,
			Random::NextFloat() * m_worldDimensions.y));
		m_food.push_back(food);
	}
}

void Simulation::UpdateSteadyStateGA()
{
	// Steady state GA for when the population is too small.
	if (m_fittestList->GetSize() > 1)
	{
		while ((int) m_agents.size() < Simulation::PARAMS.minAgents)
		{
			Agent* child = new  Agent(this);
			
			int numAgentsCreated = m_numAgentsCreatedElite + m_numAgentsCreatedMate + m_numAgentsCreatedRandom;
			
			if (Random::NextFloat() < 0.5f) // TODO: magic number
			{
				// Mate two agents.
				int iParent, jParent;
				PickParentsUsingTournament(m_fittestList->GetSize(), &iParent, &jParent);
				child->GetGenome()->Crossover(
					m_fittestList->GetByRank(iParent)->genome,
					m_fittestList->GetByRank(jParent)->genome);
				child->GetGenome()->Mutate();
				m_numAgentsCreatedMate++;
			}
			else
			{
				// Create a random agent.
				child->GetGenome()->Randomize();
				m_numAgentsCreatedRandom++;
			}
			
			// TODO: Revive a fittest agent.
			//m_numAgentsCreatedElite++;

			child->Grow();
			child->SetPosition(Vector2f(
				Random::NextFloat() * m_worldDimensions.x,
				Random::NextFloat() * m_worldDimensions.y));
			
			m_agents.push_back(child);
		}
	}
}

void Simulation::PickParentsUsingTournament(int numInPool, int* iParent, int* jParent)
{
	*iParent = numInPool-1;
	int tournamentSize = 5;
	for (int z = 0; z < tournamentSize; z++)
	{
		int r = Random::NextInt(0, numInPool);
		if (*iParent > r)
			*iParent = r;
	}

	do
	{
		*jParent = numInPool-1;
		for (int z = 0; z < tournamentSize; z++)
		{
			int r = Random::NextInt(0, numInPool);
			if (*jParent > r)
				*jParent = r;
		}
	}
	while (*jParent == *iParent);
}


//-----------------------------------------------------------------------------
// Agents.
//-----------------------------------------------------------------------------

Agent* Simulation::Mate(Agent* mommy, Agent* daddy)
{
	if ((int) m_agents.size() + 1 > Simulation::PARAMS.maxAgents)
	{
		m_numBirthsDenied++;
		std::cout << "Birth denied!" << std::endl;
		mommy->MateDelay();
		daddy->MateDelay();
		return NULL;
	}

	if (!mommy->CanMate() || mommy->GetEnergy() <= 0.0f ||
		!daddy->CanMate() || daddy->GetEnergy() <= 0.0f)
		return NULL;

	float mommyEnergy = mommy->GetEnergy() * mommy->GetGenome()->GetBirthEnergyFraction();
	float daddyEnergy = daddy->GetEnergy() * daddy->GetGenome()->GetBirthEnergyFraction();
	float childEnergy = mommyEnergy + daddyEnergy;
	
	// Mating costs energy.
	mommy->AddEnergy(-mommyEnergy);
	daddy->AddEnergy(-daddyEnergy);

	// Create the child.
	Agent* child = new Agent(this);
	child->GetGenome()->Crossover(
		mommy->GetGenome(),
		daddy->GetGenome());
	child->GetGenome()->Mutate();
	child->Grow();
	child->SetEnergy(childEnergy);
	child->SetPosition((mommy->GetPosition() + daddy->GetPosition()) * 0.5f);
	
	mommy->OnMate();
	daddy->OnMate();

	std::cout << "An agent was born!" << std::endl;
	
	m_numAgentsBorn++;

	return child;
}

void Simulation::Kill(Agent* agent)
{
	agent->SetHeuristicFitness(agent->GetHeuristicFitness() + (agent->GetAge() * Simulation::PARAMS.ageFitnessParam));
	agent->SetHeuristicFitness(agent->GetHeuristicFitness() + (agent->GetEnergy() * Simulation::PARAMS.energyFitnessParam));

	m_fittestList->Update(agent, agent->GetHeuristicFitness());
	
	std::cout << "Killing agent #" << agent->GetID() << " with fitness " << agent->GetHeuristicFitness() << std::endl;

	m_recentFitnesses.push_back(agent->GetHeuristicFitness());

	if (m_recentFitnesses.size() > 30)
		m_recentFitnesses.erase(m_recentFitnesses.begin());

	if (m_selectedAgent == agent)
		m_selectedAgent = NULL;

	delete agent;
}


//-----------------------------------------------------------------------------
// World Rendering.
//-----------------------------------------------------------------------------

void Simulation::RenderWorld(ICamera* camera, Agent* agentPOV)
{
	Graphics g(GetWindow());
	
	g.EnableCull(false); // Dont cull.
	g.EnableDepthTest(true);
	g.Clear(Color::BLACK);

	g.SetProjection(camera->GetViewProjection());
	g.ResetTransform();

	Vector4f foodColor(0.0f, 1.0f, 0.0f, 1.0f); // green
	Vector4f floorColor(0.0f, 0.15f, 0.0f, 1.0f); // dark green


	//-----------------------------------------------------------------------------
	// Draw the floor.

	//if (agentPOV == NULL)
	{
		glBegin(GL_QUADS);
		glColor4fv(&floorColor.x);
		float floorZ = -0.1f;
		glVertex3f(0.0f, 0.0f, floorZ);
		glVertex3f(Simulation::PARAMS.worldWidth, 0.0f, floorZ);
		glVertex3f(Simulation::PARAMS.worldWidth, Simulation::PARAMS.worldHeight, floorZ);
		glVertex3f(0.0f, Simulation::PARAMS.worldHeight, floorZ);
		glEnd();
	}

	//-----------------------------------------------------------------------------
	// Draw food.
	
	for (unsigned int i = 0; i < m_food.size(); i++)
	{
		Vector2f pos = m_food[i].GetPosition();
		
		g.ResetTransform();
		g.Translate(pos);

		glBegin(GL_QUADS);
		glColor4fv(&foodColor.x);
		for (unsigned int j = 0; j < m_foodVertices.size(); j++)
			glVertex3fv(m_foodVertices[j].data());

		glEnd();
	}

	//-----------------------------------------------------------------------------
	// Draw agents.

	for (unsigned int i = 0; i < m_agents.size(); i++)
	{
		Agent* agent = m_agents[i];

		if (agent == agentPOV)
			continue;

		Vector2f pos = agent->GetPosition();

		g.ResetTransform();
		g.Translate(pos);
		g.Rotate(Vector3f::UNITZ, -agent->GetDirection());
		g.Scale(agent->GetSize());

		Vector3f agentColor;
		agentColor.x = agent->GetFightAmount();
		agentColor.y = agent->GetGenome()->GetGreenColoration();
		agentColor.z = agent->GetMateAmount();

		// Draw the agent's model.
		glBegin(GL_TRIANGLES);
		glColor3fv(agentColor.data());
		for (unsigned int j = 0; j < m_foodVertices.size(); j++)
			glVertex3fv(m_agentVertices[j].data());
		glEnd();
		
		if (agentPOV == NULL)
		{
			if (m_showFOVLines)
			{
				// Draw FOV lines.
				Vector3f v1(0.0f, 0.0f, 3.0f);
				Vector3f v2(40.0f, 0.0f, 3.0f);
				Vector3f v3(40.0f, 0.0f, 3.0f);
				v2.Rotate(Vector3f::UNITZ, agent->GetFOV() * 0.5f);
				v3.Rotate(Vector3f::UNITZ, -agent->GetFOV() * 0.5f);
				glBegin(GL_LINE_STRIP);
				glColor3ub(0, 255, 255);
				glVertex3fv(v3.data());
				glVertex3fv(v1.data());
				glVertex3fv(v2.data());
				glEnd();

				// Draw vision strip between FOV lines.
				int numVisionNeurons = agent->GetRetina().GetNumNeurons(0);
				glLineWidth(3.0f);
				glBegin(GL_LINES);

				int visionWidth = 32;
				for (int j = 0; j < visionWidth; j++)
				{
					Vector3f color;
					for (int c = 0; c < agent->GetRetina().GetNumChannels(); c++)
					{
						int neuronIndex = (int) ((j / (float) visionWidth) * agent->GetRetina().GetNumNeurons(c));
						color[c] = agent->GetRetina().GetSightValue(c, neuronIndex);
					}
				
					Vector3f vv1 = Vector3f::Lerp(v3, v2, (float) j / (float) visionWidth);
					Vector3f vv2 = Vector3f::Lerp(v3, v2, (float) (j + 1) / (float) visionWidth);
					glColor3fv(color.data());
					glVertex3fv(vv1.data());
					glVertex3fv(vv2.data());
				}
				glEnd();
				glLineWidth(1.0f);
			}
			
			// Draw a selection circle.
			if (m_selectedAgent == agent)
				g.DrawCircle(Vector2f::ZERO, m_agentSelectionRadius, Color::GREEN);
		}
	}
}

// Render an agent's 1D vision.
void Simulation::RenderAgentVision(Agent* agent)
{
	Graphics g(GetWindow());

	float fovY = 0.01f;

	// Setup the camera for the agent.
	Camera agentCam;
	agentCam.projection = Matrix4f::CreatePerspectiveXY(
		agent->GetFOV(), fovY, 0.1f, 1000.0f);
	agentCam.position.SetXY(agent->GetPosition());
	agentCam.position.z = 3.0f;
	agentCam.rotation = Quaternion::IDENTITY;
	agentCam.rotation.Rotate(Vector3f::UNITZ, Math::HALF_PI);
	agentCam.rotation.Rotate(Vector3f::UNITY, Math::HALF_PI);
	agentCam.rotation.Rotate(Vector3f::UNITZ, agent->GetDirection());

	// Render the world from the agent's point-of-view.
	Viewport vp(0, 0, Simulation::PARAMS.retinaResolution, 1);
	g.SetViewport(vp, true, false);
		RenderWorld(&agentCam, agent);
	g.SetViewport(m_windowViewport, true);

	// Read the pixels that were rendered.
	Vector3f vecPixels[128];
	float pixels[128 * 3];
	glReadPixels(vp.x, vp.y, vp.width, vp.height, GL_RGB, GL_FLOAT, pixels);
	agent->UpdateVision(pixels, vp.width);
}


//-----------------------------------------------------------------------------
// Simulation Rendering
//-----------------------------------------------------------------------------

void Simulation::OnRender()
{
	Graphics g(GetWindow());
	
	// Clear the background.
	g.SetViewport(m_windowViewport, true);
	g.Clear(Color::BLACK);

	// Render the panels.
	RenderPanelWorld();
	RenderPanelPOV();
	RenderPanelText();
	RenderPanelGraphs();

	g.SetViewport(m_windowViewport, true);
	g.SetProjection(Matrix4f::CreateOrthographic(
		(float) m_windowViewport.x,
		(float) m_windowViewport.x + m_windowViewport.width,
		(float) m_windowViewport.y + m_windowViewport.height,
		(float) m_windowViewport.y,
		-1.0f, 1.0f));

	// Draw outlines around the panels.
	g.DrawRect(m_panelSide, Color::WHITE);
	g.DrawRect(m_panelPOV, Color::WHITE);
		
	// Draw the selected agent's brain's connectivity matrix.
	if (m_selectedAgent != NULL && m_showBrain)
	{
		// Rendering a brain requires a non-y-flipped projection.
		g.SetProjection(Matrix4f::CreateOrthographic(
			(float) m_windowViewport.x,
			(float) m_windowViewport.x + m_windowViewport.width,
			(float) m_windowViewport.y,
			(float) m_windowViewport.y + m_windowViewport.height,
			-1.0f, 1.0f));

		Vector2f winCenter = Vector2f(
			(float) GetWindow()->GetWidth(),
			(float) GetWindow()->GetHeight()) * 0.5f;
		m_brainRenderer.RenderBrain(&g, m_selectedAgent, winCenter);
	}
}

void Simulation::RenderPanelWorld()
{
	Graphics g(GetWindow());

	g.SetViewport(m_panelWorld, true);
	g.EnableCull(true);
	g.EnableDepthTest(true);

	g.Clear(Color::BLACK);
	
	RenderWorld(&m_camera, NULL);
}

void Simulation::RenderPanelPOV()
{
	Graphics g(GetWindow());

	g.SetViewport(m_panelPOV, true);

	g.Clear(Color::BLACK);

	if (m_selectedAgent != NULL)
	{
		Agent* agent = m_selectedAgent;

		g.EnableCull(true);
		g.EnableDepthTest(true);

		// Render the world from the agent's POV.
		Camera agentCam;
		agentCam.projection = Matrix4f::CreatePerspectiveX(agent->GetFOV(),
			m_panelPOV.GetAspectRatio(), 0.1f, 1000.0f);
		agentCam.position = Vector3f(agent->GetPosition(), 3.0f);
		agentCam.rotation = Quaternion::IDENTITY;
		agentCam.rotation.Rotate(Vector3f::UNITZ, Math::HALF_PI);
		agentCam.rotation.Rotate(Vector3f::UNITY, Math::HALF_PI);
		agentCam.rotation.Rotate(Vector3f::UNITZ, agent->GetDirection());
		RenderWorld(&agentCam, agent);
		
		g.EnableCull(false);
		g.EnableDepthTest(false);
		g.ResetTransform();
		g.Translate(Vector2f((float) m_panelPOV.x, (float) m_panelPOV.y));
		g.SetProjection(Matrix4f::CreateOrthographic(
			(float) m_panelPOV.x,
			(float) m_panelPOV.x + m_panelPOV.width,
			(float) m_panelPOV.y + m_panelPOV.height,
			(float) m_panelPOV.y,
			-1.0f, 1.0f));

		// Draw the agent's 1-dimensional vision strip.
		int numVisionNeurons = agent->GetRetina().GetNumNeurons(0);
		glBegin(GL_QUADS);

		int visionWidth			= 32;
		int pixelWidth			= 2;//m_panelPOV.width / 16;
		int visionStripheight	= 24;
		for (int x = 0; x < m_panelPOV.width; x += pixelWidth)
		{
			Vector3f color;
			for (int c = 0; c < agent->GetRetina().GetNumChannels(); c++)
			{
				float t = x / (float) m_panelPOV.width;
				//int neuronIndex = (int) ((x / (float) m_panelPOV.width) *
					//agent->GetRetina().GetNumNeurons(c));
				//color[c] = agent->GetRetina().GetSightValue(c, neuronIndex);
				color[c] = agent->GetRetina().GetInterpolatedSightValue(c, t);
			}
			
			glColor3fv(color.data());
			glVertex2f((float) x, 0.0f);
			glVertex2f((float) x + pixelWidth, 0.0f);
			glVertex2f((float) x + pixelWidth, (float) visionStripheight);
			glVertex2f((float) x, (float) visionStripheight);
		}
		glEnd();

		g.DrawLine(0.0f, (float) visionStripheight,
			(float) m_panelPOV.width, (float) visionStripheight,
			Color::WHITE);
	}
}

void Simulation::RenderPanelGraphs()
{
	Graphics g(GetWindow());

	g.SetViewport(m_panelGraphs, true);
	g.EnableCull(false);
	g.EnableDepthTest(false);
	g.Clear(Color::BLACK);
	
	g.SetProjection(Matrix4f::CreateOrthographic(
		(float) m_panelGraphs.x,
		(float) m_panelGraphs.x + m_panelGraphs.width,
		(float) m_panelGraphs.y + m_panelGraphs.height,
		(float) m_panelGraphs.y,
		-1.0f, 1.0f));
	
	g.ResetTransform();
	g.Translate(Vector2f((float) m_panelGraphs.x, (float) m_panelGraphs.y));

	m_graphEnergy.Draw(&g);
	m_graphPopulation.Draw(&g);
	//m_graphFitness.Draw(&g);
	
	glLoadIdentity();
}

void Simulation::RenderPanelText()
{
	Graphics g(GetWindow());

	g.SetViewport(m_panelText, true);
	g.EnableCull(false);
	g.EnableDepthTest(false);
	g.Clear(Color::BLACK);
	
	g.SetProjection(Matrix4f::CreateOrthographic(
		(float) m_panelText.x,
		(float) m_panelText.x + m_panelText.width,
		(float) m_panelText.y + m_panelText.height,
		(float) m_panelText.y,
		-1.0f, 1.0f));
	
	g.ResetTransform();
	g.Translate(Vector2f((float) m_panelText.x, (float) m_panelText.y));
	
	// TODO: maybe draw bars to represent percentages like age/energy/movespeed?

	//g.FillRect(Vector2f::ZERO, Vector2f(20, 20), Color::GREEN);
	
	//-----------------------------------------------------------------------------
#define DRAW_STRING(_format, ... ) \
	sprintf_s(text, _format, __VA_ARGS__);\
	g.DrawString(m_font, text, textCursor, textColor, textSize);\
	textCursor.y += textLineSpacing;
	//-----------------------------------------------------------------------------
	
	Color textColor			= Color::WHITE;
	float textSize			= 1.0f;
	float textLineSpacing	= 16.0f * textSize;
	Vector2f textCursor		= Vector2f(10, 10);
	char text[64];
	
	if (m_selectedAgent == NULL)
	{
		DRAW_STRING("SIMULATION");
		DRAW_STRING("--------------------------------");
		DRAW_STRING("age            = %d", m_worldAge);
		DRAW_STRING("energy         = %.1f", m_totalAgentEnergy);
		DRAW_STRING("energy/agent   = %.2f", m_totalAgentEnergy / (float) m_agents.size());
		DRAW_STRING("population     = %d", (int) m_agents.size());
		DRAW_STRING("food           = %d", (int) m_food.size());
		DRAW_STRING("");
		DRAW_STRING("agents born    = %d", m_numAgentsBorn);
		DRAW_STRING("agents dead    = %d", m_numAgentsDeadOldAge + m_numAgentsDeadEnergy);
		DRAW_STRING("  - old age    = %d", m_numAgentsDeadOldAge);
		DRAW_STRING("  - hunger     = %d", m_numAgentsDeadEnergy);
		DRAW_STRING("agents created = %d", m_numAgentsCreatedElite + m_numAgentsCreatedMate + m_numAgentsCreatedRandom);
		DRAW_STRING("  - elite      = %d", m_numAgentsCreatedElite);
		DRAW_STRING("  - mate       = %d", m_numAgentsCreatedMate);
		DRAW_STRING("  - random     = %d", m_numAgentsCreatedRandom);
		DRAW_STRING("births denied  = %d", m_numBirthsDenied);
		DRAW_STRING("");
		DRAW_STRING("FPS = %.1f", GetCurrentFPS());
	}
	else
	{
		Agent* agent = m_selectedAgent;
		
		char degreesSymbol = (char) 248;

		DRAW_STRING("AGENT #%lu", agent->GetID());
		DRAW_STRING("--------------------------------");
		DRAW_STRING("age              = %d (%.0f%%)",	agent->GetAge(), ((float) agent->GetAge() / agent->GetGenome()->GetLifespan()) * 100.0f);
		DRAW_STRING("energy           = %.3f (%.0f%%)",	agent->GetEnergy(), (agent->GetEnergy() / agent->GetMaxEnergy()) * 100.0f);
		DRAW_STRING("fitness          = %.2f",			agent->GetHeuristicFitness());
		DRAW_STRING("");
		DRAW_STRING("move speed       = %.2f (%.0f%%)",	agent->GetMoveSpeed(), (agent->GetMoveSpeed() / agent->GetGenome()->GetMaxSpeed()) * 100.0f);
		DRAW_STRING("turn speed       = %.2f%c",	agent->GetTurnSpeed() * Math::RAD_TO_DEG, degreesSymbol);
		DRAW_STRING("mate             = %.0f%%",	agent->GetMateAmount() * 100.0f);
		DRAW_STRING("fight            = %.0f%%",	agent->GetFightAmount() * 100.0f);
		DRAW_STRING("eat              = %.0f%%",	agent->GetEatAmount() * 100.0f);
		DRAW_STRING("");
		DRAW_STRING("food eaten       = %d",	agent->GetNumFoodEaten());
		DRAW_STRING("children         = %d",	agent->GetNumChildren());
		DRAW_STRING("");
		DRAW_STRING("--- Genome ---------------------");
		DRAW_STRING("size             = %.2f",	agent->GetSize());
		DRAW_STRING("strength         = %.2f",	agent->GetStrength());
		DRAW_STRING("fov              = %.1f%c",	agent->GetFOV() * Math::RAD_TO_DEG, degreesSymbol);
		DRAW_STRING("max speed        = %.2f",	agent->GetMaxSpeed());
		DRAW_STRING("green color      = %d",		(int) (agent->GetGenome()->GetGreenColoration() * 255.0f));
		DRAW_STRING("mutation rate    = %.2f%%",	agent->GetGenome()->GetMutationRate() * 100.0f);
		DRAW_STRING("# crossover pts  = %d",		agent->GetGenome()->GetNumCrossoverPoints());
		DRAW_STRING("lifespan         = %d",		agent->GetLifeSpan());
		DRAW_STRING("birth energy %%   = %.0f%%",	agent->GetGenome()->GetBirthEnergyFraction() * 100.0f);
		DRAW_STRING("color neurons    = %d/%d/%d",
			agent->GetGenome()->GetNumRedNeurons(),
			agent->GetGenome()->GetNumGreenNeurons(),
			agent->GetGenome()->GetNumBlueNeurons());
		DRAW_STRING("# int. groups    = %d",	agent->GetGenome()->GetNumInternalNeuralGroups());
		DRAW_STRING("# neurons        = %d",	agent->GetBrain()->GetNeuralNet()->GetDimensions().numNeurons);
		DRAW_STRING("# synapses       = %dl",	agent->GetBrain()->GetNeuralNet()->GetDimensions().numSynapses);
		DRAW_STRING("--------------------------------");
	}

	//-----------------------------------------------------------------------------
#undef DRAW_STRING
	//-----------------------------------------------------------------------------

	glLoadIdentity();
}


//-----------------------------------------------------------------------------
// OUTDATED METHODS.
//-----------------------------------------------------------------------------

void Simulation::NextGeneration()
{
	// Reset food.
	m_food.resize(Simulation::PARAMS.initialFoodCount);
	for (int i = 0; i < Simulation::PARAMS.initialFoodCount; i++)
	{
		m_food[i].SetPosition(Vector2f(
			Random::NextFloat() * m_worldDimensions.x,
			Random::NextFloat() * m_worldDimensions.y));
	}
	
	// Sort the population by fitness.
	std::sort(m_agents.begin(), m_agents.end(), [](Agent* a, Agent* b) {
		return (a->GetHeuristicFitness() > b->GetHeuristicFitness());
	});
	
	// Calculate the total fitness.
	float totalFitness = 0.0f;
	for (unsigned int i = 0; i < m_agents.size(); i++)
		totalFitness += m_agents[i]->GetHeuristicFitness();
	float averageFitness = totalFitness / (float) m_agents.size();
	
	// Print the fitness statistics for the generation.
	GenerationInfo genInfo;
	genInfo.averageFitness	= averageFitness;
	genInfo.bestFitness		= (float) m_agents.front()->GetHeuristicFitness();
	genInfo.worstFitness	= (float) m_agents.back()->GetHeuristicFitness();
	m_generationInfo.push_back(genInfo);

	m_simulationStats.clear();

	std::cout << "---------------------------------------------------" << std::endl;
	std::cout << "Generation " << m_generation << ":" << std::endl;
	std::cout << "Best fitness  = " << m_agents.front()->GetHeuristicFitness();
	if (m_agents.front()->IsElite())
		std::cout << " (elite)";
	std::cout << std::endl;
	std::cout << "Worst fitness = " << m_agents.back()->GetHeuristicFitness() << std::endl;
	std::cout << "Avg fitness   = " << averageFitness << std::endl;

	// Create new population of agents.
	std::vector<Agent*> newAgents;
	while ((int) newAgents.size() < m_numAgents - (m_numElites * m_numEliteCopies))
	{
		// Select two mates.
		Agent* mommy = AgentRoulette();
		Agent* daddy = AgentRoulette();
		Agent* child = new Agent(this);
		child->GetGenome()->Crossover(
			mommy->GetGenome(),
			daddy->GetGenome());
		child->GetGenome()->Mutate();
		child->Grow();
		child->SetPosition(Vector2f(
			Random::NextFloat() * m_worldDimensions.x,
			Random::NextFloat() * m_worldDimensions.y));
		newAgents.push_back(child);
	}

	bool isSelectedAgentElite = false;

	// Add in elites.
	for (int i = 0; i < m_numElites; i++)
	{
		Agent* elite = m_agents[i];
		
		for (int j = 0; j < m_numEliteCopies; j++)
		{
			Agent* agent = new Agent(this);
			agent->GetGenome()->CopyFrom(elite->GetGenome());
			agent->Grow();
			agent->SetElite(true);
			agent->SetPosition(Vector2f(
				Random::NextFloat() * m_worldDimensions.x,
				Random::NextFloat() * m_worldDimensions.y));
			newAgents.push_back(agent);
			
			if (m_selectedAgent == elite)
			{
				isSelectedAgentElite = true;
				m_selectedAgent = agent;
			}
		}
	}

	// Delete old generation of agents.
	for (unsigned int i = 0; i < m_agents.size(); i++)
		delete m_agents[i];

	m_agents = newAgents;
	m_generation++;

	if (!isSelectedAgentElite)
		m_selectedAgent = NULL;
}

Agent* Simulation::AgentRoulette()
{
	if (m_agents.empty())
		return NULL;

	// Calculate the total fitness.
	float totalFitness = 0.0f;
	for (int i = 0; i < m_numAgents; i++)
		totalFitness += m_agents[i]->GetHeuristicFitness();

	// Select an agent at random.
	float randomSelection = Random::NextFloat() * totalFitness;
	float fitnessCounter = 0.0f;

	for (int i = 0; i < (int) m_agents.size(); i++)
	{
		fitnessCounter += m_agents[i]->GetHeuristicFitness();

		if (randomSelection <= fitnessCounter)
			return m_agents[i];
	}

	return m_agents[0];
}

