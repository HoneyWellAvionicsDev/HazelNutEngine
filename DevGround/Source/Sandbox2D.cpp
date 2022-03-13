#include "Sandbox2D.h"
#include <imgui.h>
#include "glm/gtc/type_ptr.hpp"


Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.f / 720.f, true)
{
}

void Sandbox2D::OnAttach()
{
	HZ_PROFILE_FUNCTION();
	m_Texture = Hazel::Texture2D::Upload("assets/textures/Space.png");
}

void Sandbox2D::OnDetach()
{
	HZ_PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(Hazel::Timestep ts)
{
	HZ_PROFILE_FUNCTION();

	//------------------Update----------------------------------
	m_CameraController.OnUpdate(ts);

	//------------------Render----------------------------------
	Hazel::RenderCommand::SetClearColor({ 0.04f, 0.04f, 0.04f, 1 });
	Hazel::RenderCommand::Clear();

	Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());

	Hazel::Renderer2D::DrawQuad({ -1.f, 0.f }, { 0.8f, 0.8f }, { 0.8f, 1.f, 0.9f, 0.8f });
	Hazel::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, { 0.8f, 0.67f, 0.12f, 0.8f });
	//Hazel::Renderer2D::DrawQuad({ 0.f, 0.f, -0.f}, { 7.5f, 7.5f }, m_Texture);

	//Hazel::Renderer2D::DrawRotatedQuad({ 14.5f, -9.5f }, { 0.5f, 0.75f }, glm::radians(78.f), { 0.8f, 0.67f, 0.12f, 0.8f });
	//Hazel::Renderer2D::DrawRotatedQuad({ 5.f, 7.f, -0.1f }, { 175.5f, 175.5f }, glm::radians(45.f), m_Texture, { 0.3f, 0.97f, 0.62f, 0.95f }, 550);

	Hazel::Renderer2D::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
	//HZ_PROFILE_FUNCTION();

	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(Hazel::Event& event)
{
	m_CameraController.OnEvent(event);
}
