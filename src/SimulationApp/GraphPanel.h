#ifndef _GRAPH_PANEL_H_
#define _GRAPH_PANEL_H_

#include <AppLib/graphics/Graphics.h>
#include <vector>
#include <string>


struct GraphInfo
{

	int		dataOffset;
	int		dataStride;
	Color	color;
};



class GraphPanel
{
public:
	GraphPanel();
	
	void SetTitle(const std::string& title) { m_title = title; }
	void SetFont(SpriteFont* font) { m_font = font; }

	void SetViewBounds(float minX, float maxX, float minY, float maxY) {
		m_minX = minX;
		m_maxX = maxX;
		m_minY = minY;
		m_maxY = maxY;
	}

	void AddGraph(const GraphInfo& graph);
	void AddGraph(const Color& color, int dataOffset, int dataStride);

	void SetViewport(const Viewport& viewport);

	void SetData(float* data, int dataSize);

	void Draw(Graphics* g);

	Vector2f GetPointOnGraph(const Vector2f& point);

	void GetGraphRange(const GraphInfo& graph, float& rangeMin, float& rangeMax, float& domainMax, float& domainMin);
	void DrawGraph(Graphics* g, const GraphInfo& graph);

private:
	struct ViewBounds
	{
		Vector2f mins;
		Vector2f maxs;
	};

	std::vector<GraphInfo> m_graphs;
	Viewport	m_viewport;
	Viewport	m_graphViewport;
	
	float*		m_data;
	int			m_dataSize;

	ViewBounds	m_viewBounds;

	SpriteFont*	m_font;
	

	float		m_minY;
	float		m_maxY;
	float		m_minX;
	float		m_maxX;

	bool		m_dynamicRange;
	float		m_dynamicRangePadding;

	std::string	m_title;
};


#endif // _GRAPH_PANEL_H_