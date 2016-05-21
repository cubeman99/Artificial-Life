#include "Keyboard.h"
#include <string.h>



Keys::value_type Keyboard::TranslateSDLKeycode(SDL_Keycode keycode)
{
	switch (keycode)
	{
	// Letters.
	case SDLK_a:	return Keys::A;
	case SDLK_b:	return Keys::B;
	case SDLK_c:	return Keys::C;
	case SDLK_d:	return Keys::D;
	case SDLK_e:	return Keys::E;
	case SDLK_f:	return Keys::F;
	case SDLK_g:	return Keys::G;
	case SDLK_h:	return Keys::H;
	case SDLK_i:	return Keys::I;
	case SDLK_j:	return Keys::J;
	case SDLK_k:	return Keys::K;
	case SDLK_l:	return Keys::L;
	case SDLK_m:	return Keys::M;
	case SDLK_n:	return Keys::N;
	case SDLK_o:	return Keys::O;
	case SDLK_p:	return Keys::P;
	case SDLK_q:	return Keys::Q;
	case SDLK_r:	return Keys::R;
	case SDLK_s:	return Keys::S;
	case SDLK_t:	return Keys::T;
	case SDLK_u:	return Keys::U;
	case SDLK_v:	return Keys::V;
	case SDLK_w:	return Keys::W;
	case SDLK_x:	return Keys::X;
	case SDLK_y:	return Keys::Y;
	case SDLK_z:	return Keys::Z;
	
			// Numbers.
	case SDLK_0:	return Keys::D0;
	case SDLK_1:	return Keys::D1;
	case SDLK_2:	return Keys::D2;
	case SDLK_3:	return Keys::D3;
	case SDLK_4:	return Keys::D4;
	case SDLK_5:	return Keys::D5;
	case SDLK_6:	return Keys::D6;
	case SDLK_7:	return Keys::D7;
	case SDLK_8:	return Keys::D8;
	case SDLK_9:	return Keys::D9;
	
			// Function keys.
	case SDLK_F1:	return Keys::F1;
	case SDLK_F2:	return Keys::F2;
	case SDLK_F3:	return Keys::F3;
	case SDLK_F4:	return Keys::F4;
	case SDLK_F5:	return Keys::F5;
	case SDLK_F6:	return Keys::F6;
	case SDLK_F7:	return Keys::F7;
	case SDLK_F8:	return Keys::F8;
	case SDLK_F9:	return Keys::F9;
	case SDLK_F10:	return Keys::F10;
	case SDLK_F11:	return Keys::F11;
	case SDLK_F12:	return Keys::F12;
	case SDLK_F13:	return Keys::F13;
	case SDLK_F14:	return Keys::F14;
	case SDLK_F15:	return Keys::F15;
	case SDLK_F16:	return Keys::F16;
	case SDLK_F17:	return Keys::F17;
	case SDLK_F18:	return Keys::F18;
	case SDLK_F19:	return Keys::F19;
	case SDLK_F20:	return Keys::F20;
	case SDLK_F21:	return Keys::F21;
	case SDLK_F22:	return Keys::F22;
	case SDLK_F23:	return Keys::F23;
	case SDLK_F24:	return Keys::F24;

			// Control keys.
	case SDLK_LSHIFT:			return Keys::LSHIFT;
	case SDLK_RSHIFT:			return Keys::RSHIFT;
	case SDLK_LCTRL:			return Keys::LCONTROL;
	case SDLK_RCTRL:			return Keys::RCONTROL;
	case SDLK_LALT:				return Keys::LALT;
	case SDLK_RALT:				return Keys::RALT;
	case SDLK_LGUI:				return Keys::LWIN;
	case SDLK_RGUI:				return Keys::RWIN;
	case SDLK_SPACE:			return Keys::SPACE;
	case SDLK_RETURN:			return Keys::ENTER;
	case SDLK_BACKSPACE:		return Keys::BACKSPACE;
	case SDLK_TAB:				return Keys::TAB;
	case SDLK_CAPSLOCK:			return Keys::CAPSLOCK;
	case SDLK_CLEAR:			return Keys::CLEAR;
	case SDLK_ESCAPE:			return Keys::ESCAPE;
	case SDLK_PRINTSCREEN:		return Keys::PRINT_SCREEN;
	case SDLK_SYSREQ:			return Keys::SYSREQ;
	case SDLK_SCROLLLOCK:		return Keys::SCROLL_LOCK;
	case SDLK_PAUSE:			return Keys::PAUSE;

			// Symbol keys.
	case SDLK_PLUS:				return Keys::PLUS;
	case SDLK_MINUS:			return Keys::MINUS;
	case SDLK_EQUALS:			return Keys::EQUALS;
	case SDLK_UNDERSCORE:		return Keys::UNDERSCORE;
	case SDLK_RIGHTBRACKET:		return Keys::RIGHT_BRACKET;
	case SDLK_LEFTBRACKET:		return Keys::LEFT_BRACKET;
	case SDLK_BACKSLASH:		return Keys::BACKSLASH;
	case SDLK_SLASH:			return Keys::SLASH;
	case SDLK_QUESTION:			return Keys::QUESTION_MARK;
	case SDLK_COMMA:			return Keys::COMMA;
	case SDLK_PERIOD:			return Keys::PERIOD;
	case SDLK_LESS:				return Keys::LESS_THAN;
	case SDLK_GREATER:			return Keys::GREATER_THAN;
	case SDLK_COLON:			return Keys::COLON;
	case SDLK_SEMICOLON:		return Keys::SEMICOLON;
	case SDLK_QUOTE:			return Keys::QUOTE;
	case SDLK_QUOTEDBL:			return Keys::DOUBLE_QUOTE;
	case SDLK_KP_VERTICALBAR:	return Keys::VERTICLE_BAR;
	case SDLK_EXCLAIM:			return Keys::EXCLAMATION;
	case SDLK_BACKQUOTE:		return Keys::GRAVE; // Tilde/accent key

			// Arrow pad keys.
	case SDLK_INSERT:	return Keys::INSERT;
	case SDLK_DELETE:	return Keys::DELETE_KEY;
	case SDLK_HOME:		return Keys::HOME;
	case SDLK_END:		return Keys::END;
	case SDLK_PAGEUP:	return Keys::PAGEUP;
	case SDLK_PAGEDOWN:	return Keys::PAGEDOWN;
	case SDLK_UP:		return Keys::UP;
	case SDLK_DOWN:		return Keys::DOWN;
	case SDLK_LEFT:		return Keys::LEFT;
	case SDLK_RIGHT:	return Keys::RIGHT;

			// Numpad.
	case SDLK_KP_0:			return Keys::NUMPAD_0;
	case SDLK_KP_1:			return Keys::NUMPAD_1;
	case SDLK_KP_2:			return Keys::NUMPAD_2;
	case SDLK_KP_3:			return Keys::NUMPAD_3;
	case SDLK_KP_4:			return Keys::NUMPAD_4;
	case SDLK_KP_5:			return Keys::NUMPAD_5;
	case SDLK_KP_6:			return Keys::NUMPAD_6;
	case SDLK_KP_7:			return Keys::NUMPAD_7;
	case SDLK_KP_8:			return Keys::NUMPAD_8;
	case SDLK_KP_9:			return Keys::NUMPAD_9;
	case SDLK_KP_DECIMAL:	return Keys::NUMPAD_DECIMAL;
	case SDLK_KP_ENTER:		return Keys::NUMPAD_ENTER;
	case SDLK_KP_PLUS:		return Keys::NUMPAD_PLUS;
	case SDLK_KP_MINUS:		return Keys::NUMPAD_MINUS;
	case SDLK_KP_DIVIDE:	return Keys::NUMPAD_DIVIDE;
	case SDLK_KP_MULTIPLY:	return Keys::NUMPAD_MULTIPLY;

	default: return Keys::NONE;
	};

}



Keyboard::Keyboard()
{
	memset(m_state.keys, 0, sizeof(m_state.keys));
	memset(m_statePrev.keys, 0, sizeof(m_state.keys));
}

Keyboard::~Keyboard()
{
}

void Keyboard::Update()
{
	memcpy(m_statePrev.keys, m_state.keys, sizeof(m_state.keys));
}

bool Keyboard::IsKeyDown(keycode_type key)
{
	return m_state.keys[key];
}

bool Keyboard::IsKeyPressed(keycode_type key)
{
	return (m_state.keys[key] && !m_statePrev.keys[key]);
}

bool Keyboard::IsKeyReleased(keycode_type key)
{
	return (m_statePrev.keys[key] && !m_state.keys[key]);
}

void Keyboard::InjectKeyPress(Keys::value_type key)
{
	m_state.keys[key] = true;
}

void Keyboard::InjectKeyRelease(Keys::value_type key)
{
	m_state.keys[key] = false;
}
