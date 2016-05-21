#include "Application.h"
#include <stdio.h>
#include "util/Timing.h"


Application::Application()
	: m_isRunning(false)
{
}

Application::~Application()
{
	SDL_Quit();
}


bool Application::Initialize(const char* title, int width, int height)
{
	if (!m_window.Initialize(title, width, height))
		return false;

	OnInitialize();

	return true;
}

void Application::Run()
{
	// Main application loop.

	m_currentFPS	= 60;
	m_frameTime		= 1.0f / 60.0f;
	m_isRunning		= true;
	
	double startTime	= Time::GetTime();
	double lastTime		= startTime;
	double renderTime	= startTime;
	double frameTime	= 1.0 / 60.0;
	int    frames		= 0;

	double newTime;

	while (m_isRunning)
	{
		newTime = Time::GetTime();

		// Update FPS tracker.
		if (newTime > startTime + 1.0)
		{
			m_currentFPS = frames / (float) (newTime - startTime);
			startTime += 1.0;
			frames = 0;
		}
		
		// Update game logic.
		//Update(m_frameTime);
		//frames++;

		// Update and render 60 times per second.
		if (newTime >= renderTime + frameTime)
		{
			Update(m_frameTime);
			frames++;

			Render();

			renderTime += frameTime;
			if (newTime > renderTime + frameTime)
				renderTime = newTime + frameTime;
		}
	}
}

void Application::Update(float timeDelta)
{
	m_keyboard.Update();
	m_mouse.Update();
	
	// Handle the SDL event queue.
	SDL_Event e;
	while (SDL_PollEvent(&e))
		HandleSDLEvent(&e);
	
	OnUpdate(timeDelta);
}

void Application::HandleSDLEvent(SDL_Event* e)
{
	// Close button was pressed.
	if (e->type == SDL_QUIT)
		m_isRunning = false;
		
	// Window Events.
	else if (e->type == SDL_WINDOWEVENT)
	{
		// Window resized.
		if (e->window.event == SDL_WINDOWEVENT_RESIZED)
		{
			m_window.OnResize();
		}
	}

	// Mouse was moved.
	if (e->type == SDL_MOUSEMOTION)
	{
	}
	
	// Mouse wheel was scrolled.
	else if (e->type == SDL_MOUSEWHEEL)
	{
		m_mouse.InjectScroll(e->wheel.y);
	}

	// Key was pressed/typed.
	else if (e->type == SDL_KEYDOWN)
	{
		m_keyboard.InjectKeyPress(Keyboard::TranslateSDLKeycode(e->key.keysym.sym));
	}

	// Key was released.
	else if (e->type == SDL_KEYUP)
	{
		m_keyboard.InjectKeyRelease(Keyboard::TranslateSDLKeycode(e->key.keysym.sym));
	}

	// Mouse button was pressed.
	else if (e->type == SDL_MOUSEBUTTONDOWN)
	{
		m_mouse.InjectButtonPress(Mouse::TranslateSDLMouseButton(e->button.button));
	}

	// Mouse button was released.
	else if (e->type == SDL_MOUSEBUTTONUP)
	{
		m_mouse.InjectButtonRelease(Mouse::TranslateSDLMouseButton(e->button.button));
	}
}

void Application::Render()
{
	OnRender();
	m_window.SwapWindow();
}

void Application::Quit()
{
	m_isRunning = false;
}
