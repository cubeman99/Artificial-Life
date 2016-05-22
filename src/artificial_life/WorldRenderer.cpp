#include "WorldRenderer.h"
#include "Simulation.h"


WorldRenderer::WorldRenderer(Simulation* simulation)
	: m_simulation(simulation)
{
}

void WorldRenderer::LoadModels()
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

void WorldRenderer::RenderWorld(Graphics* g, ICamera* camera, Agent* agentPOV)
{
	g->EnableCull(false); // Dont cull.
	g->EnableDepthTest(true);
	g->Clear(Color::BLACK);

	g->SetProjection(camera->GetViewProjection());
	g->ResetTransform();

	Vector4f foodColor(0.0f, 1.0f, 0.0f, 1.0f); // green
	Vector4f floorColor(0.0f, 0.15f, 0.0f, 1.0f); // dark green
	
	//-----------------------------------------------------------------------------
	// Draw the floor.

	glBegin(GL_QUADS);
	glColor4fv(floorColor.data());
	float floorZ = -0.1f;
	glVertex3f(0.0f, 0.0f, floorZ);
	glVertex3f(Simulation::PARAMS.worldWidth, 0.0f, floorZ);
	glVertex3f(Simulation::PARAMS.worldWidth, Simulation::PARAMS.worldHeight, floorZ);
	glVertex3f(0.0f, Simulation::PARAMS.worldHeight, floorZ);
	glEnd();

	//-----------------------------------------------------------------------------
	// Draw food.
	
	for (auto it = m_simulation->food_begin(); it < m_simulation->food_end(); ++it)
	{
		Food food = *it;

		g->ResetTransform();
		g->Translate(food.GetPosition());

		glBegin(GL_QUADS);
		glColor4fv(&foodColor.x);
		for (unsigned int j = 0; j < m_foodVertices.size(); j++)
			glVertex3fv(m_foodVertices[j].data());

		glEnd();
	}

	//-----------------------------------------------------------------------------
	// Draw agents.

	for (auto it = m_simulation->agents_begin(); it < m_simulation->agents_end(); ++it)
	{
		Agent* agent = *it;

		// Don't render the host agent.
		if (agent == agentPOV)
			continue;

		g->ResetTransform();
		g->Translate(agent->GetPosition());
		g->Rotate(Vector3f::UNITZ, -agent->GetDirection());
		g->Scale(agent->GetSize());

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
	}
}

void WorldRenderer::RenderAgent(Graphics* g, Agent* agent)
{
	g->ResetTransform();
	g->Translate(agent->GetPosition());
	g->Rotate(Vector3f::UNITZ, -agent->GetDirection());
	g->Scale(agent->GetSize());

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
}

void WorldRenderer::RenderFood(Graphics* g, Food* food)
{
	Color foodColor = Color::GREEN;

	g->ResetTransform();
	g->Translate(food->GetPosition());

	glBegin(GL_QUADS);
	glColor4ubv(foodColor.data());
	for (unsigned int j = 0; j < m_foodVertices.size(); j++)
		glVertex3fv(m_foodVertices[j].data());

	glEnd();
}

void WorldRenderer::RenderAgent(Graphics* g, const Vector2f& pos, float direction, float size, const Color& color)
{
	g->ResetTransform();
	g->Translate(pos);
	g->Rotate(Vector3f::UNITZ, -direction);
	g->Scale(size);

	// Draw the agent's model.
	glBegin(GL_TRIANGLES);
	glColor4ubv(color.data());
	for (unsigned int j = 0; j < m_foodVertices.size(); j++)
		glVertex3fv(m_agentVertices[j].data());
	glEnd();
}

void WorldRenderer::RenderFood(Graphics* g, const Vector2f& pos)
{
	Color foodColor = Color::GREEN;

	g->ResetTransform();
	g->Translate(pos);

	glBegin(GL_QUADS);
	glColor4ubv(foodColor.data());
	for (unsigned int j = 0; j < m_foodVertices.size(); j++)
		glVertex3fv(m_foodVertices[j].data());

	glEnd();
}
