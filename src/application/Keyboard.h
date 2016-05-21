#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include <SDL2/SDL.h>


struct Keys
{
	typedef int value_type;

	enum
	{
		NONE = 0,

		// Letters.
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
	
		// Numbers.
		D0, D1, D2, D3, D4, D5, D6, D7, D8, D9,
	
		// Function keys.
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
		F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24,

		// Control keys.
		LSHIFT, RSHIFT, LCONTROL, RCONTROL, LALT, RALT, LWIN, RWIN,
		SPACE, ENTER, BACKSPACE, TAB, CAPSLOCK, CLEAR, ESCAPE,
		PRINT_SCREEN, SYSREQ, SCROLL_LOCK, PAUSE,

		// Symbol keys.
		PLUS, MINUS, EQUALS, UNDERSCORE,
		RIGHT_BRACKET, LEFT_BRACKET, BACKSLASH, VERTICLE_BAR,
		SLASH, QUESTION_MARK, COMMA, PERIOD, LESS_THAN, GREATER_THAN,
		COLON, SEMICOLON, QUOTE, DOUBLE_QUOTE, EXCLAMATION,
		GRAVE, // Grave = tilde/accent key

		// Arrow pad keys.
		UP, DOWN, LEFT, RIGHT,
		INSERT, DELETE_KEY, HOME, END, PAGEUP, PAGEDOWN,

		// Numpad.
		NUMPAD_0, NUMPAD_1, NUMPAD_2, NUMPAD_3, NUMPAD_4,
		NUMPAD_5, NUMPAD_6, NUMPAD_7, NUMPAD_8, NUMPAD_9,
		NUMPAD_PLUS, NUMPAD_MINUS, NUMPAD_DIVIDE, NUMPAD_MULTIPLY,
		NUMPAD_ENTER, NUMPAD_DECIMAL,
		
		COUNT
	};
};


class Keyboard
{
public:
	friend class Application;

	typedef Keys::value_type keycode_type;

public:
	Keyboard();
	~Keyboard();

	void Update();

	bool IsKeyDown(keycode_type key);
	bool IsKeyPressed(keycode_type key);
	bool IsKeyReleased(keycode_type key);

	void InjectKeyPress(keycode_type key);
	void InjectKeyRelease(keycode_type key);

private:
	static keycode_type TranslateSDLKeycode(SDL_Keycode keycode);

	struct KeyboardState
	{
		bool keys[Keys::COUNT];
	};

	KeyboardState m_state;
	KeyboardState m_statePrev;
};


#endif // _KEYBOARD_H_