#ifndef _BRAIN_RENDERER_H_
#define _BRAIN_RENDERER_H_

#include <AppLib/graphics/Graphics.h>
#include <ArtificialLife/agent/Agent.h>


class BrainRenderer
{
public:
	BrainRenderer();
	
	void RenderBrain(Graphics* g, Agent* agent, const Vector2f& position);

private:

};


#endif // _BRAIN_RENDERER_H_