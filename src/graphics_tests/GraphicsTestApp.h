#ifndef _GRAPHICS_TEST_APP_H_
#define _GRAPHICS_TEST_APP_H_

#include "application/Application.h"
#include "application/SpriteFont.h"
#include "application/Renderer.h"
#include "application/Shader.h"
#include "math/Quaternion.h"
#include "math/Vector2f.h"
#include "math/Vector3f.h"
#include "math/Vector4f.h"
#include "math/Matrix4f.h"
#include <vector>


struct AgentModel
{
	float		direction;
	Vector2f	position;
	float		size;
};

class GraphicsTestApp : public Application
{
public:
	GraphicsTestApp();
	~GraphicsTestApp();

protected:
	void OnInitialize() override;
	void OnUpdate(float timeDelta) override;
	void OnRender() override;

private:
	Renderer* m_renderer;
	Shader* m_shader;
	RenderParams m_renderParams3D;
	RenderParams m_renderParams2D;
	
	std::vector<AgentModel> m_agents;
	std::vector<Vector3f> m_agentVertices;

	

	Vector3f		m_cameraPosition;
	float			m_cameraDistance;
	Quaternion		m_cameraRotation;
};


#endif // _GRAPHICS_TEST_APP_H_