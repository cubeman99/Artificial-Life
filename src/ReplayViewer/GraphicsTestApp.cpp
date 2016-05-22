#include "GraphicsTestApp.h"
#include <AppLib/math/Matrix4f.h>
#include <AppLib/util/Random.h>
#include <AppLib/math/MathLib.h>


GraphicsTestApp::GraphicsTestApp()
	: m_shader(NULL)
{
	m_renderer = new Renderer();
}

GraphicsTestApp::~GraphicsTestApp()
{
	delete m_renderer; m_renderer = NULL;
	delete m_shader; m_shader = NULL;
}


void GraphicsTestApp::OnInitialize()
{
	//-----------------------------------------------------------------------------

	RenderParams renderParams = m_renderer->GetRenderParams();
	renderParams.SetClearColor(Color::BLACK);
	renderParams.SetClearBits(ClearBits::COLOR_BUFFER_BIT | ClearBits::DEPTH_BUFFER_BIT);
	renderParams.EnableCullFace(false);
	m_renderer->SetRenderParams(renderParams);

	m_renderParams2D = m_renderer->GetRenderParams();
	renderParams.SetClearColor(Color::BLACK);
	renderParams.SetClearBits(ClearBits::COLOR_BUFFER_BIT | ClearBits::DEPTH_BUFFER_BIT);
	renderParams.EnableCullFace(false);
	renderParams.EnableDepthTest(true);
	renderParams.EnableDepthBufferWrite(true);
	
	m_renderParams3D = m_renderer->GetRenderParams();
	renderParams.SetClearColor(Color::BLACK);
	renderParams.SetClearBits(ClearBits::COLOR_BUFFER_BIT | ClearBits::DEPTH_BUFFER_BIT);
	renderParams.EnableCullFace(false);
	renderParams.EnableDepthTest(false);
	renderParams.EnableDepthBufferWrite(false);
	
	m_renderer->SetRenderParams(m_renderParams3D);

	//-----------------------------------------------------------------------------

	std::string codeVS =
		"#version 330 core" "\n"
		"" "\n"
		"in vec3 a_vertPos;" "\n"
		//"in vec2 a_vertTexCoord;" "\n"
		"" "\n"
		//"out vec2 v_texCoord;" "\n"
		"" "\n"
		"uniform mat4 u_mvp;" "\n"
		"" "\n"
		"void main()" "\n"
		"{" "\n"
		"	gl_Position = u_mvp * vec4(a_vertPos, 1.0);" "\n"
		//"	v_texCoord = a_vertTexCoord;" "\n"
		"}" "\n"
	;
	std::string codeFS =
		"#version 330 core" "\n"
		"" "\n"
		//"in vec2 v_texCoord;" "\n"
		"" "\n"
		"out vec4 o_color;" "\n"
		"" "\n"
		//"uniform sampler2D s_texture;" "\n"
		"uniform vec4 u_color;" "\n"
		"" "\n"
		"void main()" "\n"
		"{" "\n"
		//"	o_color = texture2D(s_texture, v_texCoord);" "\n"
		"	o_color = u_color;" "\n"
		"}" "\n"
	;

	m_shader = new Shader();
	m_shader->AddStage(codeVS, ShaderType::VERTEX_SHADER);
	m_shader->AddStage(codeFS, ShaderType::FRAGMENT_SHADER);
	m_shader->CompileAndLink();

	//-----------------------------------------------------------------------------
	// Create random agent models.

	for (int i = 0; i < 100; i++)
	{
		AgentModel agent;
		agent.direction = Random::NextFloat() * Math::TWO_PI;
		agent.position = Vector2f(
			Random::NextFloat() * 800.0f,
			Random::NextFloat() * 800.0f);
		agent.size = Random::NextFloat(0.5f, 2.0f);
		m_agents.push_back(agent);
	}

	//-----------------------------------------------------------------------------
	// Create agent model.
	{
		float ah = 5.0f; // agent height.
		Vector3f v1( 10.0f,  0.0f, 0.0f);
		Vector3f v2(-10.0f, -9.0f, 0.0f);
		Vector3f v3(-10.0f,  9.0f, 0.0f);
		Vector3f v4( 10.0f,  0.0f, ah);
		Vector3f v5(-10.0f, -9.0f, ah);
		Vector3f v6(-10.0f,  9.0f, ah);

		m_agentVertices.push_back(v1); // Bottom
		m_agentVertices.push_back(v2);
		m_agentVertices.push_back(v3);
		m_agentVertices.push_back(v4); // Top
		m_agentVertices.push_back(v6);
		m_agentVertices.push_back(v5);
		m_agentVertices.push_back(v1); // Left
		m_agentVertices.push_back(v4);
		m_agentVertices.push_back(v5);
		m_agentVertices.push_back(v1);
		m_agentVertices.push_back(v5);
		m_agentVertices.push_back(v2);
		m_agentVertices.push_back(v3); // Right
		m_agentVertices.push_back(v6);
		m_agentVertices.push_back(v4);
		m_agentVertices.push_back(v3);
		m_agentVertices.push_back(v4);
		m_agentVertices.push_back(v1);
		m_agentVertices.push_back(v2); // Back
		m_agentVertices.push_back(v5);
		m_agentVertices.push_back(v6);
		m_agentVertices.push_back(v2);
		m_agentVertices.push_back(v6);
		m_agentVertices.push_back(v3);
	}

	m_cameraPosition = Vector3f(400.0f, 400.0f, 0.0f);
	m_cameraDistance = 900;
	m_cameraRotation = Quaternion::IDENTITY;
}

void GraphicsTestApp::OnUpdate(float timeDelta)
{
	Keyboard* keyboard = GetKeyboard();
	Mouse* mouse = GetMouse();

	if (keyboard->IsKeyPressed(Keys::ESCAPE))
	{
		Quit();
	}
	

	Vector3f cameraForwardMove = m_cameraRotation.GetUp() + m_cameraRotation.GetForward();
	cameraForwardMove.SetXY(cameraForwardMove.GetXY().Normalize());
	cameraForwardMove.z = 0.0f;
	Vector3f cameraRightMove = m_cameraRotation.GetRight();
	
	float minCamMoveSpeed	= 100.0f;
	float maxCamMoveSpeed	= 2500.0f;
	float minCamDist		= 20.0f;
	float maxCamDist		= 1500.0f;
	float camRotateSpeed	= 2.0f;
	float camRotateRadians	= 0.006f;

	float camZoomPercent	= 1.0f - ((m_cameraDistance - minCamDist) / (maxCamDist - minCamDist));
	float camMoveSpeed		= Math::Lerp(maxCamMoveSpeed, minCamMoveSpeed, camZoomPercent);
	float camMousePanAmount	= camMoveSpeed / 1000.0f;
	
	// WASD or Middle Mouse: Move camera.
	if (keyboard->IsKeyDown(Keys::W))
		m_cameraPosition += cameraForwardMove * camMoveSpeed * timeDelta;
	if (keyboard->IsKeyDown(Keys::S))
		m_cameraPosition -= cameraForwardMove * camMoveSpeed * timeDelta;
	if (keyboard->IsKeyDown(Keys::D))
		m_cameraPosition += cameraRightMove * camMoveSpeed * timeDelta;
	if (keyboard->IsKeyDown(Keys::A))
		m_cameraPosition -= cameraRightMove * camMoveSpeed * timeDelta;
	if (mouse->IsButtonDown(MouseButtons::MIDDLE))
	{
		m_cameraPosition -= cameraRightMove   * camMousePanAmount * (float) mouse->GetDeltaX();
		m_cameraPosition += cameraForwardMove * camMousePanAmount * (float) mouse->GetDeltaY();
	}

	// Arrow Keys or Left Mouse: Rotate camera.
	if (keyboard->IsKeyDown(Keys::NUMPAD_6)) // right
		m_cameraRotation.Rotate(Vector3f::UNITZ, camRotateSpeed * timeDelta);
	if (keyboard->IsKeyDown(Keys::NUMPAD_4)) // left
		m_cameraRotation.Rotate(Vector3f::UNITZ, -camRotateSpeed * timeDelta);
	if (keyboard->IsKeyDown(Keys::NUMPAD_8)) // up
		m_cameraRotation.Rotate(m_cameraRotation.GetRight(), -camRotateSpeed * timeDelta);
	if (keyboard->IsKeyDown(Keys::NUMPAD_5)) // down
		m_cameraRotation.Rotate(m_cameraRotation.GetRight(), camRotateSpeed * timeDelta);
	if (mouse->IsButtonDown(MouseButtons::RIGHT))
	{
		m_cameraRotation.Rotate(Vector3f::UNITZ, camRotateRadians * mouse->GetDeltaX());
		m_cameraRotation.Rotate(m_cameraRotation.GetRight(), camRotateRadians * mouse->GetDeltaY());
	}

	// Scroll Wheel or Right Mouse: Zoom in/out.
	if (mouse->GetDeltaZ() != 0)
		m_cameraDistance = m_cameraDistance * powf(0.9f, (float) mouse->GetDeltaZ());
	if (keyboard->IsKeyDown(Keys::NUMPAD_PLUS))
		m_cameraDistance = m_cameraDistance * powf(0.1f, timeDelta);
	if (keyboard->IsKeyDown(Keys::NUMPAD_MINUS))
		m_cameraDistance = m_cameraDistance * powf(0.1f, -timeDelta);
	m_cameraDistance = Math::Clamp(m_cameraDistance, minCamDist, maxCamDist);

}

void GraphicsTestApp::OnRender()
{
	//-----------------------------------------------------------------------------
	m_renderer->SetRenderParams(m_renderParams3D);
	m_renderer->ApplyRenderSettings(true);

	float windowWidth  = (float) GetWindow()->GetWidth();
	float windowHeight = (float) GetWindow()->GetHeight();

	Color color = Color::DARK_GREEN;
	Vector3f v1(0.0f, 0.0f, 0.0f);
	Vector3f v2(800.0f, 0.0f, 0.0f);
	Vector3f v3(800.0f, 800.0f, 0.0f);
	Vector3f v4(0.0f, 800.0f, 0.0f);

	float aspect = (float) GetWindow()->GetWidth() / (float) GetWindow()->GetHeight();

	Matrix4f projection = Matrix4f::CreatePerspective(1.2f, aspect, 1.0f, 2000.0f);
	//Vector3f camPos = Vector3f(400.0f, 400.0f, 900.0f);
	//Matrix4f view = Matrix4f::CreateTranslation(-camPos);

	Vector3f camPos = m_cameraPosition - (m_cameraRotation.GetForward() * m_cameraDistance);
	Matrix4f view = Matrix4f::CreateRotation(m_cameraRotation.GetConjugate()) *
					Matrix4f::CreateTranslation(-camPos);
	
	m_renderer->SetProjectionMatrix(projection);
	m_renderer->SetViewMatrix(view);

	Vector4f colorVec = color.ToVector4f();

	m_renderer->SetShader(m_shader);

	Matrix4f model = Matrix4f::IDENTITY;
	m_renderer->SetModelMatrix(model);
	Matrix4f mvp = m_renderer->GetMVP();

	//Matrix4f projection = Matrix4f::CreateOrthographic(0.0f, windowWidth, windowHeight, 0.0f, -1.0f, 1.0f);
	//glUniform1i(m_shader->GetUniform("s_texture")->GetLocation(), 0);
	glUniform4fv(m_shader->GetUniform("u_color")->GetLocation(), 1, colorVec.data());
	glUniformMatrix4fv(m_shader->GetUniform("u_mvp")->GetLocation(), 1, GL_TRUE, mvp.data());

	glBegin(GL_QUADS);
	glColor4ubv(color.data());
	glVertex3fv(v1.data());
	glVertex3fv(v2.data());
	glVertex3fv(v3.data());
	glVertex3fv(v4.data());
	glEnd();
	
	// Render agents.
	for (unsigned int i = 0; i < m_agents.size(); i++)
	{
		AgentModel agent = m_agents[i];

		model = Matrix4f::IDENTITY;
		model *= Matrix4f::CreateTranslation(Vector3f(agent.position, 0.0f));
		model *= Matrix4f::CreateRotation(Vector3f::UNITZ, -agent.direction);
		model *= Matrix4f::CreateScale(agent.size);
		m_renderer->SetModelMatrix(model);

		mvp = m_renderer->GetMVP();

		colorVec = Vector4f(0.0f, 1.0f, 1.0f, 1.0f);
		

		glUniform4fv(m_shader->GetUniform("u_color")->GetLocation(), 1, colorVec.data());
		glUniformMatrix4fv(m_shader->GetUniform("u_mvp")->GetLocation(), 1, GL_TRUE, mvp.data());

		glBegin(GL_TRIANGLES);
		for (unsigned int k = 0; k < m_agentVertices.size(); k++)
			glVertex3fv(m_agentVertices[k].data());
		glEnd();
	}

	
	//-----------------------------------------------------------------------------
	m_renderer->SetRenderParams(m_renderParams2D);
	m_renderer->ApplyRenderSettings();

	projection = Matrix4f::CreateOrthographic(0.0f, windowWidth, windowHeight, 0.0f, -1.0f, 1.0f);
	m_renderer->SetModelMatrix(Matrix4f::IDENTITY);
	m_renderer->SetViewMatrix(Matrix4f::IDENTITY);
	m_renderer->SetProjectionMatrix(projection);
	mvp = m_renderer->GetMVP();
	colorVec = Vector4f(1.0f, 0.0f, 0.0f, 1.0f);
	glUniform4fv(m_shader->GetUniform("u_color")->GetLocation(), 1, colorVec.data());
	glUniformMatrix4fv(m_shader->GetUniform("u_mvp")->GetLocation(), 1, GL_TRUE, mvp.data());

	Vector2f v5(100.0f, 100.0f);
	Vector2f v6(500.0f, 100.0f);
	Vector2f v7(500.0f, 300.0f);
	Vector2f v8(100.0f, 300.0f);
	
	glBegin(GL_QUADS);
	glColor4ubv(color.data());
	glVertex2fv(v5.data());
	glVertex2fv(v6.data());
	glVertex2fv(v7.data());
	glVertex2fv(v8.data());
	glEnd();
}

