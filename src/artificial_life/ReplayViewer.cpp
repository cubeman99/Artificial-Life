#include "ReplayViewer.h"
#include "math/Matrix4f.h"
#include "util/Random.h"
#include "math/MathLib.h"
#include "application/Graphics.h"
#include <assert.h>


ReplayViewer::ReplayViewer()
	: m_worldRenderer(NULL)
	, m_font(NULL)
{
}

ReplayViewer::~ReplayViewer()
{
	delete m_font; m_font = NULL;
	m_file.close();
}


void ReplayViewer::OnInitialize()
{
	
	m_font = new SpriteFont("../assets/font_console.png", 16, 8, 12, 0);
	m_worldRenderer.LoadModels();
		
	glDepthMask(true);
    glEnable(GL_DEPTH_CLAMP);

	//-----------------------------------------------------------------------------

	m_file.open("../replays/replay.alrp", std::ios::in | std::ios::binary);

	assert(m_file.is_open());

	// Read the header.
	m_file.read((char*) &m_replayHeader, sizeof(ReplayHeader));

	m_worldDimensions.x = m_replayHeader.worldWidth;
	m_worldDimensions.y = m_replayHeader.worldHeight;

	// Create a list of the file offsets for each frame.
	unsigned int frameOffset = sizeof(ReplayHeader);
	for (int i = 0; i < m_replayHeader.numFrames; i++)
	{
		m_file.seekg(frameOffset, std::ios::beg);
		
		ReplayFrameHeader frameHeader;
		m_file.read((char*) &frameHeader, sizeof(ReplayFrameHeader));
		m_frameFileOffsets.push_back(frameOffset);
		frameOffset += frameHeader.sizeInBytes;
	}

	SetFrame(0);

	//-----------------------------------------------------------------------------
			
	m_camera.position.x = m_worldDimensions.x * 0.5f;
	m_camera.position.y = m_worldDimensions.y * 0.5f;
	m_camera.position.z = 0.0f;
	m_camera.rotation.SetIdentity();
	
	m_cameraFOV = 1.2f;

	m_camera.projection	= Matrix4f::CreatePerspective(m_cameraFOV, GetWindow()->GetAspectRatio(), 4.0f, 3000.0f);

	float z = (float) (m_worldDimensions.y * 1.05f) / (2.0f * Math::Tan(m_cameraFOV * 0.5f));
	m_camera.distance = z;
}

void ReplayViewer::SetFrame(int frameIndex)
{
	if (frameIndex >= m_replayHeader.numFrames)
	{
		std::cout << "End of file!" << std::endl;
		return;
	}
	
	m_frameIndex = frameIndex;
	
	// Read the frame header.
	ReplayFrameHeader frameHeader;
	m_file.seekg(m_frameFileOffsets[m_frameIndex], std::ios::beg);
	m_file.read((char*) &frameHeader, sizeof(ReplayFrameHeader));
	
	// Read agents.
	m_agents.resize(frameHeader.numAgents);
	for (int i = 0; i < frameHeader.numAgents; i++)
		m_file.read((char*) &m_agents[i], sizeof(ReplayAgent));
	
	// Read food.
	m_food.resize(frameHeader.numFood);
	for (int i = 0; i < frameHeader.numFood; i++)
		m_file.read((char*) &m_food[i], sizeof(ReplayFood));
		
	m_worldAge = frameHeader.worldAge;
}

void ReplayViewer::OnUpdate(float timeDelta)
{
	Keyboard* keyboard = GetKeyboard();
	Mouse* mouse = GetMouse();

	if (keyboard->IsKeyPressed(Keys::ESCAPE))
	{
		Quit();
	}
	
	if (keyboard->IsKeyDown(Keys::RIGHT) && m_frameIndex + 1 < m_replayHeader.numFrames)
		SetFrame(m_frameIndex + 1);
	if (keyboard->IsKeyDown(Keys::LEFT) && m_frameIndex > 0)
		SetFrame(m_frameIndex - 1);
	
	if (keyboard->IsKeyDown(Keys::SPACE))
	{
		if (m_frameIndex + 1 < m_replayHeader.numFrames)
			SetFrame(m_frameIndex + 1);
	}
	
	if (keyboard->IsKeyDown(Keys::BACKSPACE))
	{
		SetFrame(0);
	}

	m_camera.projection	= Matrix4f::CreatePerspective(m_cameraFOV, GetWindow()->GetAspectRatio(), 4.0f, 3000.0f);
	
	Vector3f cameraForwardMove = m_camera.rotation.GetUp() + m_camera.rotation.GetForward();
	cameraForwardMove.SetXY(cameraForwardMove.GetXY().Normalize());
	cameraForwardMove.z = 0.0f;
	Vector3f cameraRightMove = m_camera.rotation.GetRight();
	
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

}

void ReplayViewer::OnRender()
{
	float windowWidth  = (float) GetWindow()->GetWidth();
	float windowHeight = (float) GetWindow()->GetHeight();

	
	Graphics g(GetWindow());

	g.EnableCull(false);
	g.EnableDepthTest(true);
	
	g.SetProjection(m_camera.GetViewProjection());
	g.ResetTransform();

	g.Clear(Color::BLACK);

	// Render floor.
	Vector4f floorColor(0.0f, 0.15f, 0.0f, 1.0f); // dark green
	glBegin(GL_QUADS);
	glColor4fv(floorColor.data());
	float floorZ = -0.1f;
	glVertex3f(0.0f, 0.0f, floorZ);
	glVertex3f(m_replayHeader.worldWidth, 0.0f, floorZ);
	glVertex3f(m_replayHeader.worldWidth, m_replayHeader.worldHeight, floorZ);
	glVertex3f(0.0f, m_replayHeader.worldHeight, floorZ);
	glEnd();

	// Render agents.
	for (unsigned int i = 0; i < m_agents.size(); i++)
	{
		ReplayAgent& agent = m_agents[i];
		m_worldRenderer.RenderAgent(&g,
			Vector2f(agent.x, agent.y),
			agent.direction,
			agent.size,
			Color(agent.red, agent.green, agent.blue));
	}
	
	// Render food.
	for (unsigned int i = 0; i < m_food.size(); i++)
	{
		ReplayFood& food = m_food[i];
		m_worldRenderer.RenderFood(&g,
			Vector2f(food.x, food.y));
	}
	
	g.SetProjection(Matrix4f::CreateOrthographic(0.0f, windowWidth, windowHeight, 0.0f, -1.0f, 1.0f));
	g.ResetTransform();
	
	float percent = ((float) m_frameIndex / ((float) m_replayHeader.numFrames - 1)) * 100.0f;
	char text[32];
	sprintf_s(text, "age = %d (%.0f%%)", m_worldAge, percent);
	g.DrawString(m_font, text, Vector2f(24, 24), Color::WHITE, 1.0f);
}

