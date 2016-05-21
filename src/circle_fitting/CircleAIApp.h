#ifndef _CIRCLE_AI_APP_H_
#define _CIRCLE_AI_APP_H_

#include "application/Application.h"
#include "application/SpriteFont.h"
#include "CircleGenome.h"
#include <vector>


// A simple application that uses genetic algorithms to find the
// largets circle that fits in an area without being obstructed.

namespace circle_fitting
{

struct Circle
{
	float radius;
	Vector2f center;
};

class CircleAIApp : public Application
{
public:
	CircleAIApp();

protected:
	void OnInitialize() override;
	void OnUpdate(float timeDelta) override;
	void OnRender() override;

	void ResetArea(int numObstacles, int populationSize);

	Circle GetCircle(const Genome& genome);

	float GetFitness(const Genome& genome);

	void NextGeneration();
	
	void GrabNBest(int numBest, const int numCopies, std::vector<Genome> &output);

private:

	Genome m_bestGenome;
	
	Genome m_bestGenomeCurrentGen;
	
	std::vector<Genome> m_population;

	std::vector<Circle> m_circles;
	Vector2f m_dimensions;

	int m_numObstacles;
	int m_numElites;
	int m_numEliteCopies;
	int m_populationSize;
	float m_crossoverRate;
	float m_mutationRate;
};

};

#endif // _CIRCLE_AI_APP_H_