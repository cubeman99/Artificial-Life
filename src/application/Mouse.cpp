#include "Mouse.h"
#include <string.h>


MouseButtons::value_type Mouse::TranslateSDLMouseButton(Uint8 button)
{
	switch (button)
	{
	case SDL_BUTTON_LEFT:	return MouseButtons::LEFT;
	case SDL_BUTTON_MIDDLE:	return MouseButtons::MIDDLE;
	case SDL_BUTTON_RIGHT:	return MouseButtons::RIGHT;
	case SDL_BUTTON_X1:		return MouseButtons::MOUSE4;
	case SDL_BUTTON_X2:		return MouseButtons::MOUSE5;

	default: return MouseButtons::NONE;
	};
}



Mouse::Mouse()
{
	memset(m_state.buttons, 0, sizeof(m_state.buttons));
	memset(m_statePrev.buttons, 0, sizeof(m_state.buttons));
}

Mouse::~Mouse()
{
}

void Mouse::Update()
{
	memcpy(m_statePrev.buttons, m_state.buttons, sizeof(m_state.buttons));

	m_statePrev.x = m_state.x;
	m_statePrev.y = m_state.y;
	m_statePrev.z = m_state.z;

	SDL_GetMouseState(&m_state.x, &m_state.y);
	
	// TODO: Scroll wheel.
}

bool Mouse::IsButtonDown(button_type button) const
{
	return m_state.buttons[button];
}

bool Mouse::IsButtonPressed(button_type button) const
{
	return (m_state.buttons[button] && !m_statePrev.buttons[button]);
}

bool Mouse::IsButtonReleased(button_type button) const
{
	return (m_statePrev.buttons[button] && !m_state.buttons[button]);
}

int Mouse::GetX() const
{
	return m_state.x;
}

int Mouse::GetY() const
{
	return m_state.y;
}

int Mouse::GetZ() const
{
	return m_state.z;
}

int Mouse::GetDeltaX() const
{
	return (m_state.x - m_statePrev.x);
}

int Mouse::GetDeltaY() const
{
	return (m_state.y - m_statePrev.y);
}

int Mouse::GetDeltaZ() const
{
	return (m_state.z - m_statePrev.z);
}
	
void Mouse::InjectButtonPress(button_type button)
{
	m_state.buttons[button] = true;
}

void Mouse::InjectButtonRelease(button_type button)
{
	m_state.buttons[button] = false;
}

void Mouse::InjectScroll(int scrollAmount)
{
	m_state.z += scrollAmount;
}
