#ifndef _FOOD_H_
#define _FOOD_H_

#include <AppLib/math/Vector2f.h>


class Food
{
public:
	Food()
		: m_position(0.0f, 0.0f)
		, m_energyValue(1.0f)
	{}
	
	void SetPosition(const Vector2f& pos) { m_position = pos; }
	Vector2f GetPosition() const { return m_position; }

	float GetEnergyValue() const { return m_energyValue; }
	void SetEnergyValue(float energyValue) { m_energyValue = energyValue; }

private:
	Vector2f	m_position;
	float		m_energyValue;
};


#endif // _FOOD_H_