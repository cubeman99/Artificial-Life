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

struct Bounds
{
	Vector2f mins;
	Vector2f maxs;

	Bounds()
		: mins(0, 0)
		, maxs(0, 0)
	{}

	static Bounds Union(const Bounds& a, const Bounds& b);
};

class Graph
{
public:
	Graph(std::string name, const Color& color);

	const std::string&	GetName()	const { return m_name; }
	const Color&		GetColor()	const { return m_color; }
	const Bounds&		GetBounds()	const { return m_bounds; }

	int		GetDataCount()		const { return (int) m_data.size(); }
	float	GetData(int index)	const { return m_data[index]; }
	
	void AddData(float data);

private:
	Color				m_color;
	std::string			m_name;
	std::vector<float>	m_data;
	Bounds				m_bounds;
};


class GraphPanel
{
public:
	GraphPanel();
	~GraphPanel();

	void SetTitle(const std::string& title) { m_title = title; }
	void SetFont(SpriteFont* font) { m_font = font; }

	void SetViewport(const Viewport& viewport);
	
	void SetXBounds(float minX, float maxX);
	void SetYBounds(float minY, float maxY);
	void SetDynamicRange(bool dynamicRange, float dynamicRangePadding = 0.1f);

	Graph* AddGraph(const std::string& name, const Color& color);
	Graph* GetGraph(const std::string& name);
	Graph* GetGraph() { return m_graphs[0]; }


	void Draw(Graphics* g);

	void GetGraphRange(const GraphInfo& graph, float& rangeMin, float& rangeMax, float& domainMax, float& domainMin);

private:
	Vector2f GetPointOnGraph(const Vector2f& point);
	void DrawGraph(Graphics* g, Graph* graph);

	std::vector<Graph*> m_graphs;
	Viewport	m_viewport;
	Viewport	m_graphViewport;
	
	float*		m_data;
	int			m_dataSize;

	Bounds		m_minBounds;
	Bounds		m_bounds;

	SpriteFont*	m_font;
	
	bool		m_dynamicRange;
	float		m_dynamicRangePadding;

	std::string	m_title;
};


#endif // _GRAPH_PANEL_H_