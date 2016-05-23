#include "Food.h"
#include <AppLib/math/MathLib.h>
#include <AppLib/util/Random.h>

Food::Food()
	: m_position(0.0f, 0.0f)
	, m_energyValue(1.0f)
	, m_minSize(0.4f)
	, m_maxSize(4.0f)
{
	m_size = m_maxSize;
}

void Food::Randomize()
{
	m_size = Random::NextFloat(m_minSize, m_maxSize);
}

float Food::Eat(float amount)
{
	amount = Math::Min(amount, m_size);
	m_size -= amount;

	// Eat the rest of the food if there isn't much left.
	if (m_size < 0.4f)
	{
		amount += Math::Max(0.0f, m_size);
		m_size = 0.0f;
	}

	return (amount * m_energyValue); // Return the energy given from the amount eaten.
}

bool Food::IsDepleted() const
{
	return (m_size <= 0.0f);
}

float Food::GetRadius() const
{
	return Math::Lerp(
		m_maxSize * 5.0f * 0.3f,
		m_maxSize * 5.0f,
		m_size / m_maxSize);
	//return (m_size * 5.0f);
}
