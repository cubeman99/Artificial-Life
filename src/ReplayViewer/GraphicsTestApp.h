#ifndef _GRAPHICS_TEST_APP_H_
#define _GRAPHICS_TEST_APP_H_

#include <AppLib/Application.h>
#include <AppLib/graphics/SpriteFont.h>
#include <AppLib/graphics/Renderer.h>
#include <AppLib/graphics/Shader.h>
#include <AppLib/math/Quaternion.h>
#include <AppLib/math/Vector2f.h>
#include <AppLib/math/Vector3f.h>
#include <AppLib/math/Vector4f.h>
#include <AppLib/math/Matrix4f.h>
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