#ifndef _FOOD_H_
#define _FOOD_H_

#include <AppLib/math/Vector2f.h>


class Food
{
public:
	Food();

	void Randomize();

	Vector2f	GetPosition()		const { return m_position; }
	float		GetEnergyValue()	const { return m_energyValue; }
	float		GetSize()			const { return m_size; }

	bool IsDepleted() const;

	void SetPosition(const Vector2f& pos)	{ m_position = pos; }
	void SetEnergyValue(float energyValue)	{ m_energyValue = energyValue; }
	void SetSize(float size)				{ m_size = size; }

	float Eat(float amount);


	float GetRadius() const;

private:
	Vector2f	m_position;
	float		m_energyValue;
	float		m_size;
	float		m_maxSize;
	float		m_minSize;
};


#endif // _FOOD_H_