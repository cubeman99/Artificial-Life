#ifndef _REPLAY_VIEWER_H_
#define _REPLAY_VIEWER_H_

#include <AppLib/Application.h>
#include <AppLib/graphics/SpriteFont.h>
#include <AppLib/graphics/Renderer.h>
#include <AppLib/graphics/Shader.h>
#include <AppLib/math/Quaternion.h>
#include <AppLib/math/Vector2f.h>
#include <AppLib/math/Vector3f.h>
#include <AppLib/math/Vector4f.h>
#include <AppLib/math/Matrix4f.h>
#include <ArtificialLife/WorldRenderer.h>
#include <ArtificialLife/ReplayRecorder.h>
#include <ArtificialLife/Camera.h>
#include <vector>
#include <fstream>


struct AgentState
{
	Vector2f		position;
	float			direction;
	float			size;
	unsigned char	red;
	unsigned char	green;
	unsigned char	blue;

	unsigned char	padding;
};


struct FoodState
{
	Vector2f	position;
};


class ReplayViewer : public Application
{
public:
	ReplayViewer();
	~ReplayViewer();

protected:
	void OnInitialize() override;
	void OnUpdate(float timeDelta) override;
	void OnRender() override;

	void UpdateCameraControls(float timeDelta);

	void SetFrame(int frameIndex);

private:
	SpriteFont*				m_font;

	int						m_frameIndex;

	ReplayHeader			m_replayHeader;

	float					m_cameraFOV;
	ArcBallCamera			m_arcBallCamera;
	Camera					m_fpsCamera;
	ICamera*				m_camera;

	bool					m_loop;
	bool					m_isPlaying;

	unsigned long			m_selectedAgentId;

	WorldRenderer			m_worldRenderer;

	int						m_worldAge;
	std::vector<ReplayAgent>	m_agents;
	std::vector<ReplayFood>		m_food;
	Vector2f				m_worldDimensions;

	std::fstream			m_file;

	Vector2f				m_cursorPos;
	float					m_agentSelectionRadius;
	ReplayAgent				m_selectedAgent;

	std::vector<unsigned int> m_frameFileOffsets;
};


#endif // _REPLAY_VIEWER_H_