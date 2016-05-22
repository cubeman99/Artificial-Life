#ifndef _WORLD_RENDERER_H_
#define _WORLD_RENDERER_H_

#include "Camera.h"
#include "Agent.h"
#include <vector>

class Simulation;


class WorldRenderer
{
public:
	WorldRenderer(Simulation* simulation);
	
	void LoadModels();
	void RenderWorld(ICamera* camera, Agent* agentPOV);


private:
	Simulation* m_simulation;
	
	std::vector<Vector3f> m_agentVertices;
	std::vector<Vector3f> m_foodVertices;
};


#endif // _WORLD_RENDERER_H_