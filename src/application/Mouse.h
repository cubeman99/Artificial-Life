#ifndef _MOUSE_H_
#define _MOUSE_H_

#include <SDL2/SDL.h>


struct MouseButtons
{
	typedef int value_type;

	enum
	{
		NONE = -1,
		LEFT,
		MIDDLE,
		RIGHT,
		MOUSE4,
		MOUSE5,

		COUNT
	};
};


class Mouse
{
public:
	friend class Application;

	typedef MouseButtons::value_type button_type;

public:
	Mouse();
	~Mouse();

	void Update();
	
	bool IsButtonDown(button_type button) const;
	bool IsButtonPressed(button_type button) const;
	bool IsButtonReleased(button_type button) const;

	int GetX() const;
	int GetY() const;
	int GetZ() const;
	int GetDeltaX() const;
	int GetDeltaY() const;
	int GetDeltaZ() const;
	
	void InjectButtonPress(button_type button);
	void InjectButtonRelease(button_type button);
	void InjectScroll(int scrollAmount);
	
private:

	static button_type TranslateSDLMouseButton(Uint8 button);

	struct MouseState
	{
		int x, y, z;
		bool buttons[MouseButtons::COUNT];
	};

	MouseState m_state;
	MouseState m_statePrev;
};


#endif // _MOUSE_H_