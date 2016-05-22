#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <Windows.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>

#include <AppLib/graphics/Window.h>
#include <AppLib/input/Mouse.h>
#include <AppLib/input/Keyboard.h>


class Application
{
public:
	Application();
	virtual ~Application();

	bool Initialize(const char* title, int width, int height);
	void Run();
	void Update(float timeDelta);
	void Render();

	void Quit();

	inline Keyboard* GetKeyboard()	{ return &m_keyboard; }
	inline Mouse*	 GetMouse()		{ return &m_mouse; }
	inline Window*	 GetWindow()	{ return &m_window; }

	inline float GetCurrentFPS() const { return m_currentFPS; }

protected:
	virtual void OnInitialize() {}
	virtual void OnUpdate(float timeDelta) {}
	virtual void OnRender() {}
	
private:
	void HandleSDLEvent(SDL_Event* e);

	bool		m_isRunning;
	float		m_currentFPS;
	float		m_frameTime;
	Window		m_window;
	Mouse		m_mouse;
	Keyboard	m_keyboard;
};


#endif // _APPLICATION_H_