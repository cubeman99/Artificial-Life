#include "Window.h"
#include <Windows.h>
#include <GL/glew.h>
#include <stdio.h>


Window::Window()
	: m_sdlWindow(NULL)
	, m_width(0)
	, m_height(0)
{
}

Window::~Window()
{
	if (m_sdlWindow != NULL)
	{
		SDL_GL_DeleteContext(m_glContext);
		SDL_DestroyWindow(m_sdlWindow);
	}
}

void Window::SetTitle(const char* title)
{
	SDL_SetWindowTitle(m_sdlWindow, title);
}

bool Window::Initialize(const char* title, int width, int height)
{
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,   8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	
	m_sdlWindow = SDL_CreateWindow(title,
								   SDL_WINDOWPOS_CENTERED,
								   SDL_WINDOWPOS_CENTERED,
								   width, height,
								   SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	
	m_glContext = SDL_GL_CreateContext(m_sdlWindow);
	
	//SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
	//SDL_GL_SetSwapInterval(1); // VSync
	SDL_GL_SetSwapInterval(0); // Immediate updates.

	//Apparently this is necessary to build with Xcode
	glewExperimental = GL_TRUE;
	
	GLenum res = glewInit();
	if (res != GLEW_OK)
	{
		fprintf(stderr, "Glew init error: '%s'\n", glewGetErrorString(res));
		return false;
	}

	m_width  = width;
	m_height = height;

	// Setup OpenGL.

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
		
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDepthMask(false);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_CLAMP);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glViewport(0, 0, width, height);

	return true;
}

void Window::SwapWindow()
{
	SDL_GL_SwapWindow(m_sdlWindow);
}


void Window::OnResize()
{
	SDL_GetWindowSize(m_sdlWindow, &m_width, &m_height);
	glViewport(0, 0, m_width, m_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, m_width, m_height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

