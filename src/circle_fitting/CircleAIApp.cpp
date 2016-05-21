#include "CircleAIApp.h"
#include <math/Vector4f.h>
#include "util/Timing.h" 
#include "util/Random.h"
#include <algorithm> // for sort


namespace circle_fitting
{

CircleAIApp::CircleAIApp()
{
}

void CircleAIApp::OnInitialize()
{
	Random::SeedTime();

	m_dimensions = Vector2f(
		(float) GetWindow()->GetWidth(),
		(float) GetWindow()->GetHeight());


	m_numObstacles		= 30;
	m_populationSize	= 150;
	m_numElites			= 4;
	m_numEliteCopies	= 1;
	m_crossoverRate		= 0.8f;
	m_mutationRate		= 0.05f;

	// Create random circles.
	ResetArea(m_numObstacles, m_populationSize);
}

void CircleAIApp::OnUpdate(float timeDelta)
{
	Keyboard* keyboard = GetKeyboard();
	Mouse* mouse = GetMouse();
		
	// Escape: Exit the program.
	if (keyboard->IsKeyPressed(Keys::ESCAPE))
	{
		Quit();
		return;
	}

	if (keyboard->IsKeyPressed(Keys::BACKSPACE))
	{
		ResetArea(m_numObstacles, m_populationSize);
	}

	if (keyboard->IsKeyPressed(Keys::ENTER) || keyboard->IsKeyDown(Keys::SPACE))
	{
		NextGeneration();
	}

}


void CircleAIApp::ResetArea(int numObstacles, int populationSize)
{
	m_circles.resize(numObstacles);
	m_population.resize(populationSize);

	for (int i = 0; i < numObstacles; i++)
	{
		Circle circle;
		circle.radius   = 16 + (Random::NextFloat() * 32);
		//circle.center.x = circle.radius + (Random::NextFloat() * (m_dimensions.x - circle.radius * 2.0f));
		//circle.center.y = circle.radius + (Random::NextFloat() * (m_dimensions.y - circle.radius * 2.0f));
		circle.center.x = circle.radius + (Random::NextFloat() * m_dimensions.x);
		circle.center.y = circle.radius + (Random::NextFloat() * m_dimensions.y);
		m_circles[i] = circle;
	}

	for (int i = 0; i < populationSize; i++)
	{
		m_population[i] = Genome(3);
		m_population[i].Randomize();
		m_population[i].SetMutationRate(m_mutationRate);
		m_population[i].SetFitness(GetFitness(m_population[i]));
	}
	
	m_bestGenome = m_population[0];
	m_bestGenomeCurrentGen = m_population[0];
}

void CircleAIApp::NextGeneration()
{
	
	//-----------------------------------------------------------------------------
	// Find the total fitness of the population.

	float totalFitness = 0.0f;
	float bestFitness = 0.0f;
	int  bestIndex = 0;

	for (unsigned int i = 0; i < m_population.size(); i++)
	{
		float fitness = m_population[i].GetFitness();
		totalFitness += fitness;

		if (i == 0 || fitness > bestFitness)
		{
			bestFitness = fitness;
			bestIndex = i;
		}
	}

	//std::cout << "Best Fitness = " << bestFitness << ", ";
	//std::cout << "Avg. Fitness = " << totalFitness / (float) m_population.size() << std::endl;
	
	//-----------------------------------------------------------------------------
	// Move to the next generation.
	
	
	std::vector<Genome> newPopulation;
	newPopulation.resize(m_population.size() - (m_numElites * m_numEliteCopies));

	for (int k = 0; k < (int) newPopulation.size(); k++)
	{

		// Select two members from the current population.
		float randomSelection = Random::NextFloat() * totalFitness;
		float fitnessCounter = 0.0f;
		int mateIndex1 = (int) m_population.size() - 1;
		for (int i = 0; i < (int) m_population.size(); i++)
		{
			fitnessCounter += m_population[i].GetFitness();
			if (randomSelection <= fitnessCounter)
			{
				mateIndex1 = i;
				break;
			}
		}

		fitnessCounter = 0.0f;

		int mateIndex2 = (int) m_population.size() - 1;
		for (int i = 0; i < (int) m_population.size(); i++)
		{
			//if (i != mateIndex1)
			{
				fitnessCounter += (int) m_population.size();
				if (randomSelection <= fitnessCounter)
				{
					mateIndex2 = i;
					break;
				}
			}
		}

		Genome mate1 = m_population[mateIndex1];
		Genome mate2 = m_population[mateIndex2];
		Genome child(mate1.GetNumBytes());
		
		if (Random::NextFloat() < m_crossoverRate)
			child.Crossover(&mate1, &mate2, false);
		else if (Random::NextBool())
			child = mate1;
		else
			child = mate2;

		child.SetMutationRate(m_mutationRate);
		child.Mutate();
		
		child.SetFitness(GetFitness(child));
		
		if (child.GetFitness() > m_bestGenome.GetFitness())
			m_bestGenome = child;
		if (child.GetFitness() > m_bestGenomeCurrentGen.GetFitness() || k == 0)
			m_bestGenomeCurrentGen = child;

		newPopulation[k] = child;
	}

	GrabNBest(m_numElites, m_numEliteCopies, newPopulation);
		
	m_population = newPopulation;
}

void CircleAIApp::GrabNBest(int numBest, const int numCopies, std::vector<Genome> &output)
{
	sort(m_population.begin(), m_population.end());

	//now add the required amount of copies of the n most fittest 
	//to the supplied vector
	while (numBest--)
	{
		for (int i = 0; i < numCopies; ++i)
		{
			output.push_back(m_population[numBest]);
		}
	}
}

void DrawCircle(const Vector2f& pos, float radius, const Vector4f& color)
{
	int detail = 20;

	glBegin(GL_LINE_LOOP);
	glColor4fv(&color.x);

	for (int i = 0; i < detail; i++)
	{
		float angle = (i / (float) detail) * 2.0f * 3.14159265358f;
		float x = pos.x + radius * cosf(angle);
		float y = pos.y + radius * sinf(angle);
		glVertex2f(x, y);
	}

	glEnd();
}

void DrawCircle(const Circle& circle, const Vector4f& color)
{
	DrawCircle(circle.center, circle.radius, color);
}

float CircleAIApp::GetFitness(const Genome& genome)
{
	Circle circle = GetCircle(genome);

	if (circle.center.x < circle.radius || circle.center.y < circle.radius || 
		circle.center.x > m_dimensions.x - circle.radius ||
		circle.center.y > m_dimensions.y - circle.radius)
	{
		return 0.0f;
	}

	float fitness = circle.radius * circle.radius;
	
	for (unsigned int i = 0; i < m_circles.size(); i++)
	{
		// Check if colliding with obstacle circle.
		const Circle& obstacle = m_circles[i];
		if (Vector2f::Dist(circle.center, obstacle.center) < circle.radius + obstacle.radius)
		{
			return 0.0f;
			//fitness *= 0.5f;
		}
	}

	return fitness;
}

Circle CircleAIApp::GetCircle(const Genome& genome)
{
	Circle circle;
	circle.center.x = (genome.GetByte(0) / 255.0f) * m_dimensions.x;
	circle.center.y = (genome.GetByte(1) / 255.0f) * m_dimensions.x;
	circle.radius = (genome.GetByte(2) / 255.0f) * m_dimensions.x * 0.5f;
	return circle;
}

void CircleAIApp::OnRender()
{
	// Clear the background.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Setup the orthographic projection.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, (float) GetWindow()->GetWidth(), (float) GetWindow()->GetHeight(), 0, -1, 1);
    
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	
	Vector2f pos(64.0f, 128.0f);
	float radius = 16.0f;
	Vector4f obstacleColor(0.5f, 0.5f, 0.5f, 1.0f);
	Vector4f circleColor(0.0f, 1.0f, 0.0f, 1.0f);
	Vector4f bestColor(0.0f, 1.0f, 1.0f, 1.0f);


	for (unsigned int i = 0; i < m_circles.size(); i++)
	{
		DrawCircle(m_circles[i], obstacleColor);
	}

	for (unsigned int i = 0; i < m_population.size(); i++)
		DrawCircle(GetCircle(m_population[i]), circleColor);

	DrawCircle(GetCircle(m_bestGenomeCurrentGen), circleColor);
	DrawCircle(GetCircle(m_bestGenome), bestColor);
}

};