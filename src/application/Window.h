#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <SDL2/SDL.h>


class Window
{
public:
	Window();
	~Window();
	
	bool Initialize(const char* title, int width, int height);

	void SetTitle(const char* title);

	void SwapWindow();
	
	void OnResize();

	inline int GetWidth() const { return m_width; }
	inline int GetHeight() const { return m_height; }
	inline float GetAspectRatio() const { return ((float) m_width / (float) m_height); }

private:
	SDL_Window* m_sdlWindow;
	SDL_GLContext m_glContext;
	int m_width;
	int m_height;
};


#endif // _WINDOW_H_