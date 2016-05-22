#include "Mouse.h"
#include <string.h>
#include "Window.h"


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



Mouse::Mouse(Window* window)
	: m_window(window)
{
	memset(m_state.buttons, 0, sizeof(m_state.buttons));
	memset(m_statePrev.buttons, 0, sizeof(m_state.buttons));
}

Mouse::~Mouse()
{
}

void Mouse::OnFrameStart()
{
	memcpy(m_statePrev.buttons, m_state.buttons, sizeof(m_state.buttons));

	m_statePrev.x = m_state.x;
	m_statePrev.y = m_state.y;
	m_statePrev.z = m_state.z;
}

void Mouse::Update()
{
	SDL_GetMouseState(&m_state.x, &m_state.y);
}

void Mouse::SetVisible(bool isVisible)
{
	SDL_ShowCursor(isVisible ? 1 : 0);
}

void Mouse::SetPositionInWindow(int x, int y)
{
	SDL_WarpMouseInWindow(m_window->m_sdlWindow, x, y);
	m_state.x = x;
	m_state.y = y;
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
