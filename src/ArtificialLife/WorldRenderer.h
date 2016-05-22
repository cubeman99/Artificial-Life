#ifndef _WORLD_RENDERER_H_
#define _WORLD_RENDERER_H_

#include <ArtificialLife/agent/Agent.h>
#include <ArtificialLife/food/Food.h>
#include <ArtificialLife/Camera.h>
#include <AppLib/graphics/Graphics.h>
#include <vector>

class Simulation;


class WorldRenderer
{
public:
	WorldRenderer(Simulation* simulation);
	
	void LoadModels();
	void RenderWorld(Graphics* g, ICamera* camera, Agent* agentPOV);

	void RenderAgent(Graphics* g, Agent* agent);
	void RenderFood(Graphics* g, Food* food);
	
	void RenderAgent(Graphics* g, const Vector2f& pos, float direction, float size, const Color& color);
	void RenderFood(Graphics* g, const Vector2f& pos);


private:
	Simulation* m_simulation;
	
	std::vector<Vector3f> m_agentVertices;
	std::vector<Vector3f> m_foodVertices;
};


#endif // _WORLD_RENDERER_H_