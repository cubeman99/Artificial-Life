#include "GraphPanel.h"
#include "math/MathLib.h"

GraphPanel::GraphPanel()
	: m_data(NULL)
	, m_dataSize(0)
	, m_dynamicRange(true)
	, m_dynamicRangePadding(0.1f)
	, m_font(NULL)
	, m_minX(0)
	, m_maxX(10)
	, m_maxY(10)
	, m_minY(0)
{
}


void GraphPanel::AddGraph(const GraphInfo& graph)
{
	m_graphs.push_back(graph);
}

void GraphPanel::AddGraph(const Color& color, int dataOffset, int dataStride)
{
	GraphInfo graph;
	graph.color			= color;
	graph.dataOffset	= dataOffset;
	graph.dataStride	= dataStride;
	m_graphs.push_back(graph);
}

void GraphPanel::SetData(float* data, int dataSize)
{
	m_data = data;
	m_dataSize = dataSize;
}

void GraphPanel::SetViewport(const Viewport& viewport)
{
	m_viewport = viewport;
}

void GraphPanel::Draw(Graphics* g)
{
	Color m_colorBackground	= Color::BLACK;
	Color m_colorGraphArea	= Color::BLACK;
	Color m_colorOutline	= Color::WHITE;
	Color colorZeroY		= Color::WHITE;
	Color colorZeroX		= Color::WHITE;
	Color colorGraphMargin	= Color::LIGHT_GRAY;
	Color colorLabels		= Color::WHITE;
	
	//-----------------------------------------------------------------------------
	// Calculate view bounds

	m_viewBounds.mins.x = (float) m_minX;
	m_viewBounds.maxs.x = (float) m_maxX;
	m_viewBounds.mins.y = (float) m_minY;
	m_viewBounds.maxs.y = (float) m_maxY;
	
	// Fit the view to the data's range.
	if (m_dynamicRange)
	{
		// Calculate the range.
		for (unsigned int i = 0; i < m_graphs.size(); i++)
		{
			GetGraphRange(m_graphs[i],
				m_viewBounds.mins.y, m_viewBounds.maxs.y,
				m_viewBounds.mins.x, m_viewBounds.maxs.x);
		}

		// Add in some padding so the graph doesn't touch the edges.
		float range = m_viewBounds.maxs.y - m_viewBounds.mins.y;
		m_viewBounds.mins.y -= range * m_dynamicRangePadding * 0.5f;
		m_viewBounds.maxs.y += range * m_dynamicRangePadding * 0.5f;
	}

	//-----------------------------------------------------------------------------

	char labelMin[32];
	char labelMax[32];
	sprintf_s(labelMin, "%.2f", m_viewBounds.mins.y);
	sprintf_s(labelMax, "%.2f", m_viewBounds.maxs.y);
	int labelMinLength = strnlen_s(labelMin, 32);
	int labelMaxLength = strnlen_s(labelMax, 32);
	int labelTextLength = Math::Max(labelMinLength, labelMaxLength);
	
	int padding		= 6;
	int labelWidth	= 8 * labelTextLength;
	int labelHeight	= 12;
	int titleWidth	= m_title.length() * 8;
	int titleHeight	= 12;

	m_graphViewport.x		= labelWidth + (padding * 2);
	m_graphViewport.y		= padding + titleHeight;
	m_graphViewport.width	= m_viewport.width - (padding * 3) - labelWidth;
	m_graphViewport.height	= m_viewport.height - (padding * 2) - titleHeight;

	//-----------------------------------------------------------------------------
	// GRAPH BACKGROUND.

	glDisable(GL_LINE_SMOOTH);
	g->SetViewport(m_viewport, true);
	g->SetProjection(Matrix4f::CreateOrthographic(
		(float) m_viewport.x,
		(float) m_viewport.x + m_viewport.width,
		(float) m_viewport.y + m_viewport.height,
		(float) m_viewport.y,
		-1.0f, 1.0f));
	
	g->ResetTransform();
	
	// Draw the background and outline.
	g->FillRect(m_viewport, m_colorBackground);
	g->DrawRect(m_viewport, m_colorOutline);

	g->Translate(Vector2f((float) m_viewport.x, (float) m_viewport.y));
	
	// Draw the graph area's background.
	g->FillRect(m_graphViewport, m_colorGraphArea);

	// Draw the graph area's left margin.
	g->DrawLine((float) m_graphViewport.x,
				(float) m_graphViewport.y,
				(float) m_graphViewport.x,
				(float) m_graphViewport.y + m_graphViewport.height,
				colorGraphMargin);
	
	// Draw tick marks at the top and bottom of the graph area's left margin.
	int tickMarkWidth = 6;
	g->DrawLine((float) m_graphViewport.x - (tickMarkWidth / 2),
				(float) m_graphViewport.y,
				(float) m_graphViewport.x + (tickMarkWidth / 2),
				(float) m_graphViewport.y,
				colorGraphMargin);
	g->DrawLine((float) m_graphViewport.x - (tickMarkWidth / 2),
				(float) m_graphViewport.y + m_graphViewport.height,
				(float) m_graphViewport.x + (tickMarkWidth / 2),
				(float) m_graphViewport.y + m_graphViewport.height,
				colorGraphMargin);

	// Draw the y-axis labels.
	g->DrawString(m_font, labelMax,
		Vector2f((float) padding + labelWidth - (labelMaxLength * 8),
		(float) m_graphViewport.y - (labelHeight / 2)), colorLabels);
	g->DrawString(m_font, labelMin,
		Vector2f((float) padding + labelWidth - (labelMinLength * 8),
		(float) m_graphViewport.y + m_graphViewport.height - (labelHeight / 2)), colorLabels);

	g->DrawString(m_font, m_title.c_str(),
		Vector2f((float) (m_viewport.width / 2) - (titleWidth / 2),
		(float) padding), colorLabels);
	
	//-----------------------------------------------------------------------------
	// GRAPH PLOTS.

	// Draw the line for y = 0 and x = 0.
	Vector2f origin = GetPointOnGraph(Vector2f(0.0f, 0.0f));
	if (0.0f > m_viewBounds.mins.y && 0.0f < m_viewBounds.maxs.y)
		g->DrawLine((float) m_graphViewport.x, origin.y, (float) m_graphViewport.x + m_graphViewport.width, origin.y, colorZeroY);
	if (0.0f > m_viewBounds.mins.x && 0.0f < m_viewBounds.maxs.x)
		g->DrawLine(origin.x, (float) m_graphViewport.y, origin.x, (float) m_graphViewport.y + m_graphViewport.height, colorZeroX);

	// Draw the individual graphs.
	for (unsigned int i = 0; i < m_graphs.size(); i++)
	{
		DrawGraph(g, m_graphs[i]);
	}
}

Vector2f GraphPanel::GetPointOnGraph(const Vector2f& point)
{
	return Vector2f(
		m_graphViewport.x + (point.x - m_viewBounds.mins.x) / (m_viewBounds.maxs.x - m_viewBounds.mins.x) * m_graphViewport.width,
		m_graphViewport.y + m_graphViewport.height - ((point.y - m_viewBounds.mins.y) / (m_viewBounds.maxs.y - m_viewBounds.mins.y) * m_graphViewport.height));
}

void GraphPanel::GetGraphRange(const GraphInfo& graph, float& rangeMin, float& rangeMax, float& domainMin, float& domainMax)
{
	int i, x;
	for (i = graph.dataOffset, x = 0; i < m_dataSize; i += graph.dataStride, x++)
	{
		float y = m_data[i];
		if (y < rangeMin)
			rangeMin = y;
		if (y > rangeMax)
			rangeMax = y;
		if ((float) x < domainMin)
			domainMin = (float) x;
		if ((float) x > domainMax)
			domainMax = (float) x;
	}
}

void GraphPanel::DrawGraph(Graphics* g, const GraphInfo& graph)
{
	int i, x;

	glBegin(GL_LINE_STRIP);
	glColor4ubv(graph.color.data());

	for (i = graph.dataOffset, x = 0; i < m_dataSize; i += graph.dataStride, x++)
	{
		float y = m_data[i];

		if (x >= m_viewBounds.mins.x && x <= m_viewBounds.maxs.x)
		{
			Vector2f point = GetPointOnGraph(Vector2f((float) x, y));
			glVertex2fv(point.data());
		}
	}
	
	glEnd();
}
