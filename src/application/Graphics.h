#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include "Color.h"
#include "math/Vector2f.h"
#include "math/Vector3f.h"
#include "math/Vector4f.h"
#include "math/Matrix4f.h"
#include "math/Quaternion.h"
#include "Window.h"
#include "SpriteFont.h"

#include <Windows.h>
#include <GL/glew.h>


struct Viewport
{
	int x;
	int y;
	int width;
	int height;

	Viewport()
	{}

	Viewport(int x, int y, int width, int height)
		: x(x), y(y), width(width), height(height)
	{}

	float GetAspectRatio() const
	{
		return ((float) width / (float) height);
	}

	void Inset(int amount)
	{
		x += amount;
		y += amount;
		width  -= amount * 2;
		height -= amount * 2;
	}
};


class Graphics
{
public:
	Graphics(Window* window);

	void Clear(const Color& color);
	void SetViewport(const Viewport& viewport, bool scissor, bool flipY = true);

	void DrawLine(float x1, float y1, float x2, float y2, const Color& color);
	void DrawLine(const Vector2f& from, const Vector2f& to, const Color& color);
	void DrawRect(const Viewport& rect, const Color& color);
	void DrawRect(const Vector2f& pos, const Vector2f& size, const Color& color);
	void DrawRect(float x, float y, float width, float height, const Color& color);
	void FillRect(const Vector2f& pos, const Vector2f& size, const Color& color);
	void FillRect(const Viewport& rect, const Color& color);
	void FillRect(float x, float y, float width, float height, const Color& color);
	void DrawCircle(const Vector2f& pos, float radius, const Color& color, int numEdges = 20);
	void FillCircle(const Vector2f& pos, float radius, const Color& color, int numEdges = 20);
	void DrawString(SpriteFont* font, const char* text, const Vector2f& pos, const Color& color, float scale = 1.0f);

	
	void EnableCull(bool cull);
	void EnableDepthTest(bool depthTest);

	void SetProjection(const Matrix4f& projection);
	
	void ResetTransform();
	void SetTransform(const Matrix4f& transform);
	
	void Transform(const Matrix4f& transform);
	void Rotate(const Vector3f& axis, float angle);
	void Rotate(const Quaternion& rotation);
	void Translate(const Vector2f& translation);
	void Translate(const Vector3f& translation);

private:
	
	void gl_Vertex(const Vector2f& v);
	void gl_Vertex(const Vector3f& v);
	void gl_Color(const Color& color);

	Window* m_window;
};


#endif // _GRAPHICS_H_