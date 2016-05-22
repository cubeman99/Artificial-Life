#include "ReplayViewer.h"
#include <AppLib/math/Matrix4f.h>
#include <AppLib/math/MathLib.h>
#include <AppLib/util/Random.h>
#include <AppLib/graphics/Graphics.h>
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
	m_selectedAgentId = 0;
	m_agentSelectionRadius = 20.0f;

	m_font = new SpriteFont("../../assets/font_console.png", 16, 8, 12, 0);
	m_worldRenderer.LoadModels();
		
	glDepthMask(true);
    glEnable(GL_DEPTH_CLAMP);

	m_isPlaying	= false;
	m_loop		= false;

	//-----------------------------------------------------------------------------

	m_file.open("../../replays/replay.alrp", std::ios::in | std::ios::binary);

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

	m_selectedAgentId = m_agents[5].id;

	//-----------------------------------------------------------------------------
			
	m_arcBallCamera.position.x = m_worldDimensions.x * 0.5f;
	m_arcBallCamera.position.y = m_worldDimensions.y * 0.5f;
	m_arcBallCamera.position.z = 0.0f;
	m_arcBallCamera.rotation.SetIdentity();
	
	m_cameraFOV = 1.2f;

	m_arcBallCamera.projection= Matrix4f::CreatePerspective(m_cameraFOV, GetWindow()->GetAspectRatio(), 4.0f, 3000.0f);

	float z = (float) (m_worldDimensions.y * 1.05f) / (2.0f * Math::Tan(m_cameraFOV * 0.5f));
	m_arcBallCamera.distance = z;

	m_fpsCamera.position = Vector3f::ZERO;
	m_fpsCamera.rotation = Quaternion::IDENTITY;
	m_fpsCamera.projection = m_arcBallCamera.projection;

	m_camera = &m_arcBallCamera;
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

Quaternion LookRotation(const Vector3f& lookAt, const Vector3f& upDirection)
{
	//Vector3f forward = Vector3f(1, 0, -0.5f);
	//Vector3f up = Vector3f::UNITZ;

	Vector3f forward = lookAt;
	Vector3f up = upDirection;

	//Vector::OrthoNormalize(&forward, &up);
	forward.Normalize();
	up.Normalize();
	//up = Vector3f::Cross(forward.Cross(up), forward);

	Vector3f right = Vector3f::Cross(forward, up);
	up = Vector3f::Cross(right, forward);
	
	//Vector3f right = Vector3f::Cross(forward, up);
	
	//Vector3f u = -Vector3f::UNITZ;
	Vector3f u = -Vector3f::UNITZ;
	Vector3f v = lookAt;
    float m = sqrt(2.0f + 2.0f * Vector3f::Dot(u, v));
    Vector3f w = (1.0f / m) * Vector3f::Cross(u, v);
    Quaternion ret(w.x, w.y, w.z, 0.5f * m);


	/*
#define m00 right.x
#define m01 up.x
#define m02 forward.x
#define m10 right.y
#define m11 up.y
#define m12 forward.y
#define m20 right.z
#define m21 up.z
#define m22 forward.z
	
	Quaternion ret;
	ret.w = sqrtf(1.0f + m00 + m11 + m22) * 0.5f;
	float w4_recip = 1.0f / (4.0f * ret.w);
	ret.x = (m21 - m12) * w4_recip;
	ret.y = (m02 - m20) * w4_recip;
	ret.z = (m10 - m01) * w4_recip;

#undef m00
#undef m01
#undef m02
#undef m10
#undef m11
#undef m12
#undef m20
#undef m21
#undef m22
	*/

	return ret;
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
	
	if (keyboard->IsKeyDown(Keys::SPACE) || m_isPlaying)
	{
		if (m_frameIndex + 1 < m_replayHeader.numFrames)
			SetFrame(m_frameIndex + 1);
		else
			m_isPlaying = false;
	}

	// Enter: start/stop playback.
	if (keyboard->IsKeyPressed(Keys::ENTER))
	{
		if (m_isPlaying)
			m_isPlaying = false;
		else
		{
			m_isPlaying = true;
			SetFrame((m_frameIndex + 1) % m_replayHeader.numFrames);
		}
	}
	
	if (keyboard->IsKeyDown(Keys::BACKSPACE))
	{
		SetFrame(0);
	}

	m_camera->SetProjection(Matrix4f::CreatePerspective(
		m_cameraFOV, GetWindow()->GetAspectRatio(), 4.0f, 3000.0f));
	
	UpdateCameraControls(timeDelta);

	
	//-----------------------------------------------------------------------------
	// Calculate cursor world position on the ground plane.

	Vector2f winSize((float) GetWindow()->GetWidth(), (float) GetWindow()->GetHeight());
	Vector2f mouseOff((float) GetMouse()->GetX(), (float) GetMouse()->GetY());
	mouseOff -= winSize * 0.5f;
	mouseOff /= winSize * 0.5f;
	
	float m_cameraAspect = GetWindow()->GetAspectRatio();

	float	 planeDist		= 0.0f;
	Vector3f planeNormal	= Vector3f::UNITZ;
	Vector3f rayPos			= m_camera->GetViewPosition();
	Vector3f rayNormal		= m_camera->GetOrientation().GetForward();
	
	// Screen point to world ray.
	float zdist = 1.0f / Math::Tan(m_cameraFOV * 0.5f);
	rayNormal.z = -zdist;
	rayNormal.x = mouseOff.x * m_cameraAspect;
	rayNormal.y = -mouseOff.y;
	rayNormal.Normalize();
	rayNormal.Rotate(m_camera->GetOrientation());

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
		m_selectedAgentId = 0;
		float nearestAgentDist = 0.0f;
		unsigned long nearestAgentId = 0;

		// Find the agent closest to the mouse cursor.
		for (unsigned int i = 0; i < m_agents.size(); i++)
		{
			float dist = Vector2f::Dist(m_cursorPos, Vector2f(m_agents[i].x, m_agents[i].y));
			if (dist < nearestAgentDist || nearestAgentId == 0)
			{
				nearestAgentId = m_agents[i].id;
				nearestAgentDist = dist;
			}
		}

		if (nearestAgentId > 0 && nearestAgentDist < m_agentSelectionRadius)
			m_selectedAgentId = nearestAgentId;
	}

	for (unsigned int i = 0; i < m_agents.size(); i++)
	{
		if (m_selectedAgentId == m_agents[i].id)
			m_selectedAgent = m_agents[i];
	}

	if (keyboard->IsKeyDown(Keys::F))
	{
		Vector3f agentPos = Vector3f(m_selectedAgent.x, m_selectedAgent.y, 0.0f);
		Vector3f lookAt = Vector3f::Normalize(agentPos - m_fpsCamera.position);
		m_fpsCamera.rotation = LookRotation(lookAt, Vector3f::UNITZ);
	}
}

void ReplayViewer::UpdateCameraControls(float timeDelta)
{
	Keyboard* keyboard = GetKeyboard();
	Mouse* mouse = GetMouse();

	if (keyboard->IsKeyPressed(Keys::C))
	{
		if (m_camera == &m_arcBallCamera)
		{
			std::cout << "Switched to FPS camera" << std::endl;

			m_camera = &m_fpsCamera;
			
			m_fpsCamera.position	= m_arcBallCamera.GetViewPosition();
			m_fpsCamera.projection	= m_arcBallCamera.projection;
			m_fpsCamera.rotation	= m_arcBallCamera.rotation;
		}
		else
		{
			std::cout << "Switched to Arc Ball camera" << std::endl;

			m_camera = &m_arcBallCamera;
			
			m_arcBallCamera.position	= m_fpsCamera.GetViewPosition();
			m_arcBallCamera.projection	= m_fpsCamera.projection;
			m_arcBallCamera.rotation	= m_fpsCamera.rotation;
			m_arcBallCamera.distance	= 100.0f;
			
			float	 planeDist		= 0.0f;
			Vector3f planeNormal	= Vector3f::UNITZ;
			Vector3f rayPos			= m_fpsCamera.position;
			Vector3f rayNormal		= m_fpsCamera.rotation.GetForward();
	
			// Raycast onto the ground plane.
			Vector3f cameraLookPoint = Vector3f::ZERO;
			float denom = planeNormal.Dot(rayNormal);
			if (denom < -0.0001f)
			{
				float distance = Vector3f::Dot((planeNormal * planeDist) - rayPos, planeNormal) / denom; 
				m_arcBallCamera.distance = distance;
				m_arcBallCamera.position = rayPos + (rayNormal * distance);
			}
			else
			{
			}
		}
	}

	if (m_camera == &m_arcBallCamera)
	{
		Vector3f cameraForwardMove = m_arcBallCamera.rotation.GetUp() + m_arcBallCamera.rotation.GetForward();
		cameraForwardMove.SetXY(cameraForwardMove.GetXY().Normalize());
		cameraForwardMove.z = 0.0f;
		Vector3f cameraRightMove = m_arcBallCamera.rotation.GetRight();
	
		float minCamMoveSpeed	= 100.0f;
		float maxCamMoveSpeed	= 2500.0f;
		float minCamDist		= 20.0f;
		float maxCamDist		= 1500.0f;
		float camRotateSpeed	= 2.0f;
		float camRotateRadians	= 0.006f;

		float camZoomPercent	= 1.0f - ((m_arcBallCamera.distance - minCamDist) / (maxCamDist - minCamDist));
		float camMoveSpeed		= Math::Lerp(maxCamMoveSpeed, minCamMoveSpeed, camZoomPercent);
		float camMousePanAmount	= camMoveSpeed / 1000.0f;
	
		// WASD or Middle Mouse: Move camera.
		if (keyboard->IsKeyDown(Keys::W))
			m_arcBallCamera.position += cameraForwardMove * camMoveSpeed * timeDelta;
		if (keyboard->IsKeyDown(Keys::S))
			m_arcBallCamera.position -= cameraForwardMove * camMoveSpeed * timeDelta;
		if (keyboard->IsKeyDown(Keys::D))
			m_arcBallCamera.position += cameraRightMove * camMoveSpeed * timeDelta;
		if (keyboard->IsKeyDown(Keys::A))
			m_arcBallCamera.position -= cameraRightMove * camMoveSpeed * timeDelta;
		if (mouse->IsButtonDown(MouseButtons::MIDDLE))
		{
			m_arcBallCamera.position -= cameraRightMove   * camMousePanAmount * (float) mouse->GetDeltaX();
			m_arcBallCamera.position += cameraForwardMove * camMousePanAmount * (float) mouse->GetDeltaY();
		}

		// Arrow Keys or Left Mouse: Rotate camera.
		if (keyboard->IsKeyDown(Keys::NUMPAD_6)) // right
			m_arcBallCamera.rotation.Rotate(Vector3f::UNITZ, camRotateSpeed * timeDelta);
		if (keyboard->IsKeyDown(Keys::NUMPAD_4)) // left
			m_arcBallCamera.rotation.Rotate(Vector3f::UNITZ, -camRotateSpeed * timeDelta);
		if (keyboard->IsKeyDown(Keys::NUMPAD_8)) // up
			m_arcBallCamera.rotation.Rotate(m_arcBallCamera.rotation.GetRight(), -camRotateSpeed * timeDelta);
		if (keyboard->IsKeyDown(Keys::NUMPAD_5)) // down
			m_arcBallCamera.rotation.Rotate(m_arcBallCamera.rotation.GetRight(), camRotateSpeed * timeDelta);
		if (mouse->IsButtonDown(MouseButtons::RIGHT))
		{
			m_arcBallCamera.rotation.Rotate(Vector3f::UNITZ, camRotateRadians * mouse->GetDeltaX());
			m_arcBallCamera.rotation.Rotate(m_arcBallCamera.rotation.GetRight(), camRotateRadians * mouse->GetDeltaY());
			mouse->SetVisible(false);
			mouse->SetPositionInWindow(
				GetWindow()->GetWidth() / 2,
				GetWindow()->GetHeight() / 2);
		}
		else
			mouse->SetVisible(true);

		// Scroll Wheel or Right Mouse: Zoom in/out.
		if (mouse->GetDeltaZ() != 0)
			m_arcBallCamera.distance = m_arcBallCamera.distance * powf(0.9f, (float) mouse->GetDeltaZ());
		if (keyboard->IsKeyDown(Keys::NUMPAD_PLUS))
			m_arcBallCamera.distance = m_arcBallCamera.distance * powf(0.1f, timeDelta);
		if (keyboard->IsKeyDown(Keys::NUMPAD_MINUS))
			m_arcBallCamera.distance = m_arcBallCamera.distance * powf(0.1f, -timeDelta);
		m_arcBallCamera.distance = Math::Clamp(m_arcBallCamera.distance, minCamDist, maxCamDist);
	}
	else
	{
		float camMoveSpeed		= 700.0f;
		float camRotateSpeed	= 2.0f;
		float camRotateRadians	= 0.006f;
		
		// Shift: Slow camera move speed.
		if (keyboard->IsKeyDown(Keys::LSHIFT))
			camMoveSpeed *= 0.5f;
		
		// WASD: Move camera.
		if (keyboard->IsKeyDown(Keys::W))
			m_fpsCamera.position += m_fpsCamera.rotation.GetForward() * camMoveSpeed * timeDelta;
		if (keyboard->IsKeyDown(Keys::S))
			m_fpsCamera.position += m_fpsCamera.rotation.GetBack() * camMoveSpeed * timeDelta;
		if (keyboard->IsKeyDown(Keys::D))
			m_fpsCamera.position += m_fpsCamera.rotation.GetRight() * camMoveSpeed * timeDelta;
		if (keyboard->IsKeyDown(Keys::A))
			m_fpsCamera.position += m_fpsCamera.rotation.GetLeft() * camMoveSpeed * timeDelta;

		// Arrow Keys or Left Mouse: Rotate camera.
		if (keyboard->IsKeyDown(Keys::NUMPAD_6)) // right
			m_fpsCamera.rotation.Rotate(Vector3f::UNITZ, camRotateSpeed * timeDelta);
		if (keyboard->IsKeyDown(Keys::NUMPAD_4)) // left
			m_fpsCamera.rotation.Rotate(Vector3f::UNITZ, -camRotateSpeed * timeDelta);
		if (keyboard->IsKeyDown(Keys::NUMPAD_8)) // up
			m_fpsCamera.rotation.Rotate(m_fpsCamera.rotation.GetRight(), -camRotateSpeed * timeDelta);
		if (keyboard->IsKeyDown(Keys::NUMPAD_5)) // down
			m_fpsCamera.rotation.Rotate(m_fpsCamera.rotation.GetRight(), camRotateSpeed * timeDelta);
		if (mouse->IsButtonDown(MouseButtons::RIGHT))
		{
			m_fpsCamera.rotation.Rotate(Vector3f::UNITZ, camRotateRadians * mouse->GetDeltaX());
			m_fpsCamera.rotation.Rotate(m_fpsCamera.rotation.GetRight(), camRotateRadians * mouse->GetDeltaY());
			mouse->SetVisible(false);
			mouse->SetPositionInWindow(
				GetWindow()->GetWidth() / 2,
				GetWindow()->GetHeight() / 2);
		}
		else
			mouse->SetVisible(true);
	}
}

void ReplayViewer::OnRender()
{
	float windowWidth  = (float) GetWindow()->GetWidth();
	float windowHeight = (float) GetWindow()->GetHeight();

	
	Graphics g(GetWindow());

	g.EnableCull(false);
	g.EnableDepthTest(true);
	
	g.SetProjection(m_camera->GetViewProjection());
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

		// Draw selection circle.
		if (m_selectedAgentId == agent.id)
		{
			g.DrawCircle(Vector2f::ZERO, m_agentSelectionRadius, Color::GREEN);
		}
	}
	
	// Render food.
	for (unsigned int i = 0; i < m_food.size(); i++)
	{
		ReplayFood& food = m_food[i];
		m_worldRenderer.RenderFood(&g,
			Vector2f(food.x, food.y));
	}

	
	//g.ResetTransform();
	//g.Translate(m_cursorPos);
	//g.DrawCircle(Vector2f::ZERO, 4.0f, Color::YELLOW);
	
	
	g.SetProjection(Matrix4f::CreateOrthographic(0.0f, windowWidth, windowHeight, 0.0f, -1.0f, 1.0f));
	g.ResetTransform();
	
	float percent = ((float) m_frameIndex / ((float) m_replayHeader.numFrames - 1)) * 100.0f;
	char text[32];
	sprintf_s(text, "age = %d (%.0f%%)", m_worldAge, percent);
	g.DrawString(m_font, text, Vector2f(24, 24), Color::WHITE, 1.0f);
}

