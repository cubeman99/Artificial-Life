 #include "SimulationApp.h"
#include <AppLib/graphics/Graphics.h>
#include <AppLib/math/MathLib.h>
#include <AppLib/math/Vector4f.h>
#include <AppLib/util/Timing.h>
#include <ArtificialLife/brain/Brain.h>


const char* g_fontPath = "../../assets/font_console.png";
const char* g_replayPath = "../../replays/replay.alrp";



SimulationApp::SimulationApp()
	: m_font(NULL)
	, m_simulation(NULL)
	, m_replayRecorder(NULL)
	, m_brainRenderer(NULL)
{
}

SimulationApp::~SimulationApp()
{
	delete m_replayRecorder; m_replayRecorder = NULL;
	delete m_brainRenderer; m_brainRenderer = NULL;
	delete m_simulation; m_simulation = NULL;
	delete m_font; m_font = NULL;
}


void SimulationApp::OnInitialize()
{
	Random::SeedTime();

	//-----------------------------------------------------------------------------
	// Load resources.

	m_font = new SpriteFont(g_fontPath, 16, 8, 12, 0);

		
	m_simulation		= new Simulation();
	m_replayRecorder	= new ReplayRecorder(m_simulation);
	m_brainRenderer		= new BrainRenderer();

	//-----------------------------------------------------------------------------
	// Reset simulation variables.

	m_agentSelectionRadius = 20;

	m_showFOVLines			= false;
	m_showInteractionRadii	= false;
	m_showGraphs			= false;
	m_showBrain				= false;
	m_followAgent			= false;
	m_selectedAgent			= NULL;
	m_selectedAgentID		= 0;

	m_totalAgentEnergy		= 0.0f;

	m_cameraFOV				= 80.0f * Math::DEG_TO_RAD;


	SimulationParams params;

	//-----------------------------------------------------------------------------
	// Simulation globals.

	params.worldWidth				= 1000;
	params.worldHeight				= 1000;
	params.boundaryType				= BoundaryType::BOUNDARY_TYPE_WRAP;

	params.minAgents				= 45;//35;
	params.maxAgents				= 200;//150;//120;
	params.initialNumAgents			= 45;

	params.minFood					= 120;//220;
	params.maxFood					= 120;//300;
	params.initialFoodCount			= 120;//220;
		
	//-----------------------------------------------------------------------------
	// Energy and fitness parameters.
    
	params.numFittest				= 10;
	params.pairFrequency			= 100;
	params.eliteFrequency			= 2;

	// Parameters for measuring an agent's fitness.
	params.eatFitnessParam			= 1.0f;
	params.mateFitnessParam			= 10.0f;
	params.moveFitnessParam			= 1.0f / 800.0f;
	params.energyFitnessParam		= 2.0f;
	params.ageFitnessParam			= 0.03f;
	
	// Energy costs.
	params.energyCostEat			= 0.0f;
	params.energyCostMate			= 0.002f;
	params.energyCostFight			= 0.002f;
	params.energyCostMove			= 0.0005f;//0.002f;
	params.energyCostTurn			= 0.0005f; //0.002f;
	params.energyCostNeuron			= 0.0f; // TODO: find a value for this.
	params.energyCostSynapse		= 0.0f; // TODO: find a value for this.
	params.energyCostExist			= 0.0005f;

	//float maxsynapse2energy; // (amount if all synapses usable)
	//float maxneuron2energy;

	//-----------------------------------------------------------------------------
	// Agent configuration.
	
	params.mateWait					= 120;
	params.initialMateWait			= 120;
	params.retinaResolution			= 16;
	params.retinaVerticalFOV		= 0.01f;

	//-----------------------------------------------------------------------------
	// Agent gene ranges.

	params.minFOV					= 20.0f * Math::DEG_TO_RAD;
	params.maxFOV					= 130.0f * Math::DEG_TO_RAD;
	params.minStrength				= 0.0f;
	params.maxStrength				= 1.0f;
	params.minSize					= 0.7f;
	params.maxSize					= 1.6f;
	params.minMaxSpeed				= 1.0f;
	params.maxMaxSpeed				= 2.5f;
	params.minMutationRate			= 0.01f;
	params.maxMutationRate			= 0.1f;
	params.minNumCrossoverPoints	= 2;
	params.maxNumCrossoverPoints	= 6; // supposed to be 8
	params.minLifeSpan				= 1500;
	params.maxLifeSpan				= 2800;
	params.minBirthEnergyFraction	= 0.1f;
	params.maxBirthEnergyFraction	= 0.7f;
	params.minVisNeuronsPerGroup	= 1;
	params.maxVisNeuronsPerGroup	= 16;
	params.minInternalNeuralGroups	= 1;
	params.maxInternalNeuralGroups	= 5;

	params.minENeuronsPerGroup		= 1;
	params.maxENeuronsPerGroup		= 6;
	params.minINeuronsPerGroup		= 1;
	params.maxINeuronsPerGroup		= 6; // supposed to be 16

	params.minConnectionDensity		= 0.0f;
	params.maxConnectionDensity		= 1.0f;
	params.minTopologicalDistortion	= 0.0f;
	params.maxTopologicalDistortion	= 1.0f;
	params.minSynapseLearningRate	= 0.0f;
	params.maxSynapseLearningRate	= 0.1f;
	
	//-----------------------------------------------------------------------------
	// Brain configuration.

	params.numInputNeurGroups		= 5; // red, green, blue, energy, random
	params.numOutputNeurGroups		= 5; // speed, turn, mate, fight, eat (MISSING focus and light).
	params.numPrebirthCycles		= 10;
	params.maxBias					= 1.0f;
	params.minBiasLearningRate		= 0.0f; // unused
	params.maxBiasLearningRate		= 0.1f; // unused
	params.logisticSlope			= 1.0f;
	params.maxWeight				= 1.0f;
	params.initMaxWeight			= 0.5f;
	params.decayRate				= 0.99f;
	
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

	m_graphPopulation.SetViewBounds(0, 120,
		//(float) (params.minAgents / 2),
		//(float) ((int) (params.maxAgents * 1.2f)));
		(float) params.minAgents,
		(float) params.maxAgents);
	m_graphPopulation.SetDynamicRange(false);

	//-----------------------------------------------------------------------------

	// Setup OpenGL state.
	glDepthMask(true);
    glEnable(GL_DEPTH_CLAMP);
	

	// Initialize the simulation.
	m_simulation->Initialize(params);
	
	UpdateScreenLayout();
	ResetCamera();
}

void SimulationApp::ResetCamera()
{
	m_camera.position.x = Simulation::PARAMS.worldWidth * 0.5f;
	m_camera.position.y = Simulation::PARAMS.worldHeight * 0.5f;
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

void SimulationApp::OnUpdate(float timeDelta)
{
	// Update the simulation.
	m_simulation->Update();
	
	// Check if our selected agent has died.
	// TODO: make an event queue for simultaion (for births and deaths)
	if (m_selectedAgent != NULL && m_simulation->GetAgent(m_selectedAgentID) == NULL)
	{
		m_selectedAgent = NULL;
		m_selectedAgentID = 0;
	}

	UpdateControls(timeDelta);
	UpdateScreenLayout();
	UpdateStatistics();
	
	if (m_replayRecorder->IsRecording())
		m_replayRecorder->RecordStep();
}

void SimulationApp::UpdateControls(float timeDelta)
{
	Keyboard* keyboard = GetKeyboard();
	Mouse* mouse = GetMouse();
	
	//-----------------------------------------------------------------------------
	// Simulation controls.
		
	// Escape: Exit the program.
	if (keyboard->IsKeyDown(Keys::LCONTROL) && keyboard->IsKeyPressed(Keys::Q))
	{
		Quit();
		return;
	}

	// F5: Start/stop recording a replay.
	if (keyboard->IsKeyPressed(Keys::F5))
	{
		if (m_replayRecorder->IsRecording())
		{
			m_replayRecorder->StopRecording();
			std::cout << " ****** RECORDING STOPPED ******" << std::endl;
		}
		else
		{
			m_replayRecorder->BeginRecording(g_replayPath);
			std::cout << " ****** RECORDING STARTED ******" << std::endl;
		}
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
	
	// B: Show/hide agent FOV/vision lines.
	if (keyboard->IsKeyPressed(Keys::I))
		m_showInteractionRadii = !m_showInteractionRadii;
	
	// G: Follow selected agent.
	if (keyboard->IsKeyPressed(Keys::F))
		m_followAgent = !m_followAgent;

	//-----------------------------------------------------------------------------
	// Camera controls.

	Vector3f cameraForwardMove = m_camera.rotation.GetUp() + m_camera.rotation.GetForward();
	cameraForwardMove.SetXY(cameraForwardMove.GetXY().Normalize());
	cameraForwardMove.z = 0.0f;
	Vector3f cameraRightMove = m_camera.rotation.GetRight();

	// TODO: magic numbers.
	float minCamMoveSpeed	= 100.0f;
	float maxCamMoveSpeed	= 4000.0f;
	float minCamDist		= 20.0f;
	float maxCamDist		= 2500.0f;
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
		mouse->SetVisible(false);
		mouse->SetPositionInWindow(
			m_panelWorld.x + (m_panelWorld.width / 2),
			m_panelWorld.y + (m_panelWorld.height / 2));
	}
	else
	{
		mouse->SetVisible(true);
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
		m_selectedAgentID = 0;
		float nearestAgentDist = 0.0f;
		Agent* nearestAgent = NULL;

		// Find the agent closest to the mouse cursor.
		for (auto it = m_simulation->agents_begin(); it != m_simulation->agents_end(); ++it)
		{
			Agent* agent = *it;
			float dist = Vector2f::Dist(m_cursorPos, agent->GetPosition());
			if (dist < nearestAgentDist || nearestAgent == NULL)
			{
				nearestAgent = agent;
				nearestAgentDist = dist;
			}
		}

		if (nearestAgent != NULL && nearestAgentDist < m_agentSelectionRadius)
		{
			m_selectedAgent = nearestAgent;
			m_selectedAgentID = nearestAgent->GetID();
		}
	}
}

void SimulationApp::UpdateScreenLayout()
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

void SimulationApp::UpdateStatistics()
{
	// Total energy.
	m_totalAgentEnergy = 0.0f;
	for (auto it = m_simulation->agents_begin(); it != m_simulation->agents_end(); ++it)
	{
		Agent* agent = *it;
		m_totalAgentEnergy  += (*it)->GetEnergy();
	}

	// Update world statistics.
	if (m_simulation->GetWorldAge() % 60 == 0)
	{
		float totalEnergy = 0.0f;
		for (auto it = m_simulation->agents_begin(); it != m_simulation->agents_end(); ++it)
			totalEnergy += (*it)->GetEnergy();
		
		Stats stats;
		stats.totalEnergy = totalEnergy;
		m_simulationStats.push_back(stats);
		m_populationData.push_back((float) m_simulation->GetNumAgents());
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
		m_graphFitness.SetData((float*) &m_generationInfo[0], (int) m_generationInfo.size() * 4);
	}
}


//-----------------------------------------------------------------------------
// Simulation Rendering
//-----------------------------------------------------------------------------

void SimulationApp::OnRender()
{
	Graphics g(GetWindow());

	m_simulation->RenderAgentsVision(&g);
	
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

	// Draw recording notification.
	if (m_replayRecorder->IsRecording())
	{
		g.DrawString(m_font, "RECORDING", Vector2f(16, 16), Color::RED, 2.0f);
	}
		
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
		m_brainRenderer->RenderBrain(&g, m_selectedAgent, winCenter);
	}
}

void SimulationApp::RenderPanelWorld()
{
	Graphics g(GetWindow());

	g.SetViewport(m_panelWorld, true);
	g.EnableCull(true);
	g.EnableDepthTest(true);

	g.Clear(Color::BLACK);
	
	// Render the world.
	m_simulation->GetWorldRenderer()->RenderWorld(&g, &m_camera, NULL);
		
	// Draw a circle around the selected agent.
	if (m_selectedAgent != NULL)
	{
		g.ResetTransform();
		g.Translate(m_selectedAgent->GetPosition());
		g.Rotate(Vector3f::UNITZ, -m_selectedAgent->GetDirection());
		g.Scale(m_selectedAgent->GetSize());
		g.DrawCircle(Vector2f::ZERO, m_agentSelectionRadius, Color::GREEN);
	}
	
	// Draw circles around food.
	if (m_showInteractionRadii)
	{
		for (auto it = m_simulation->food_begin(); it != m_simulation->food_end(); ++it)
		{
			Food* food = *it;
			g.ResetTransform();
			g.Translate(food->GetPosition());
			g.DrawCircle(Vector2f::ZERO, food->GetRadius(), Color::GREEN);
		}
	}

	// Draw lines for FOV and vision.
	if (m_showFOVLines || m_showInteractionRadii)
	{
		for (auto it = m_simulation->agents_begin(); it != m_simulation->agents_end(); ++it)
		{
			Agent* agent = *it;

			if (m_showInteractionRadii)
			{
				g.ResetTransform();
				g.Translate(agent->GetPosition());
				g.Rotate(Vector3f::UNITZ, -agent->GetDirection());
			
				g.DrawCircle(Vector2f::ZERO, agent->GetEatRadius(), Color::YELLOW);
				g.DrawCircle(Vector2f::ZERO, agent->GetMateRadius(), Color::CYAN);
				g.DrawCircle(Vector2f::ZERO, agent->GetFightRadius(), Color::RED);
			}

			if (m_showFOVLines)
			{
				g.ResetTransform();
				g.Translate(agent->GetPosition());
				g.Rotate(Vector3f::UNITZ, -agent->GetDirection());
				g.Scale(agent->GetSize());
				
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
		}
	}
}

void SimulationApp::RenderPanelPOV()
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
		m_simulation->GetWorldRenderer()->RenderWorld(&g, &agentCam, agent);
		
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

void SimulationApp::RenderPanelGraphs()
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

void SimulationApp::RenderPanelText()
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
		SimulationStats stats = m_simulation->GetStatistics();

		DRAW_STRING("SIMULATION");
		DRAW_STRING("--------------------------------");
		DRAW_STRING("age            = %d", m_simulation->GetWorldAge());
		DRAW_STRING("size           = %.0fx%.0f", Simulation::PARAMS.worldWidth, Simulation::PARAMS.worldHeight);
		DRAW_STRING("energy         = %.1f", m_totalAgentEnergy);
		DRAW_STRING("energy/agent   = %.2f", m_totalAgentEnergy / (float) m_simulation->GetNumAgents());
		DRAW_STRING("population     = %d", (int) m_simulation->GetNumAgents());
		DRAW_STRING("food           = %d", (int) m_simulation->GetNumFood());
		DRAW_STRING("");
		DRAW_STRING("agents born    = %d", stats.numAgentsBorn);
		DRAW_STRING("agents dead    = %d", stats.numAgentsDeadOldAge + stats.numAgentsDeadEnergy);
		DRAW_STRING("  - old age    = %d", stats.numAgentsDeadOldAge);
		DRAW_STRING("  - hunger     = %d", stats.numAgentsDeadEnergy);
		DRAW_STRING("agents created = %d", stats.numAgentsCreatedElite + stats.numAgentsCreatedMate + stats.numAgentsCreatedRandom);
		DRAW_STRING("  - elite      = %d", stats.numAgentsCreatedElite);
		DRAW_STRING("  - mate       = %d", stats.numAgentsCreatedMate);
		DRAW_STRING("  - random     = %d", stats.numAgentsCreatedRandom);
		DRAW_STRING("births denied  = %d", stats.numBirthsDenied);
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
}

