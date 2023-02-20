#include "SceneHierarchyPanel.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/ScriptableEntity.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <cstring>

/* The Microsoft C++ compiler is non-compliant with the C++ standard and needs
 * the following definition to disable a security warning on std::strncpy().
 */
#ifdef _MSVC_LANG
#define _CRT_SECURE_NO_WARNINGS
#endif


namespace Hazel
{
	const std::filesystem::path g_AssetPath = "assets";

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
		m_SelectionContext = {};
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		if (m_Context)
		{
			ImGui::Text("Scene: %s", m_Context->GetSceneName().string().c_str());
			static ImGuiTextFilter filter;
			filter.Draw("Search");
			
			//ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());

			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
				m_SelectionContext = {};

			if (ImGui::BeginPopupContextWindow(0, 1, false)) //TODO: only display this menu if SceneState == Edit
			{
				if (ImGui::MenuItem("Create Blank Entity"))
						m_Context->CreateEntity("Blank Entity");


				ImGui::EndPopup();
			}

			//m_Context->m_Registry.each([&](auto entityID)
			//{
			//	Entity entity{(entt::entity)((m_Context->m_Registry.size() - 1) - (int)entityID), m_Context.get() };
			//	if(filter.PassFilter(entity.GetName().c_str()))
			//		DrawEntityNode(entity);
			//});

			auto view = m_Context->GetAllEntitiesWith<IDComponent>();
			for (auto e : view)
			{
				Entity entity{ e, m_Context.get() };
				if (filter.PassFilter(entity.GetName().c_str()))
					DrawEntityNode(entity);
			}
		}

		ImGui::End();
		//DeleteDeffered();
		ImGui::Begin("Properties");
		if (m_SelectionContext)
		{
			DrawComponents(m_SelectionContext);
		}
		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked())
			m_SelectionContext = entity;
		bool deleted = false;
		if (ImGui::BeginPopupContextItem()) //TODO: only display this menu if SceneState == Edit (we dont wanna delete entities during runtime)
		{
			if (ImGui::MenuItem("Delete Entity"))
				deleted = true; 

			ImGui::EndPopup();
		}

		if (opened)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;
			ImGui::TreePop();
		}

		if (deleted)
		{
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
		}
	}

	static void DrawVec3Control(const std::string& label, glm::vec3& values, float sliderSpeedfloat, float resetValue = 0.f, float columnWidth = 100.f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.f;
		ImVec2 buttonSize = { lineHeight + 3.f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.4f, 0.05f, 0.075f, 1.f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopStyleColor(3);
		ImGui::PopFont();

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, sliderSpeedfloat, 0.f, 0.f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.35f, 0.1f, 1.f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopStyleColor(3);
		ImGui::PopFont();

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, sliderSpeedfloat, 0.f, 0.f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.05f, 0.125f, 0.4f, 1.f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopStyleColor(3);
		ImGui::PopFont();

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, sliderSpeedfloat, 0.f, 0.f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
	}

	static void DrawVec2Control(const std::string& label, glm::vec2& values, float sliderSpeedfloat, float resetValue = 0.f, float columnWidth = 100.f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.f;
		ImVec2 buttonSize = { lineHeight + 3.f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.4f, 0.05f, 0.075f, 1.f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopStyleColor(3);
		ImGui::PopFont();

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, sliderSpeedfloat, 0.f, 0.f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.35f, 0.1f, 1.f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopStyleColor(3);
		ImGui::PopFont();

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, sliderSpeedfloat, 0.f, 0.f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& label, Entity entity, UIFunction function)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4,4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.f;
			ImGui::Separator();
			bool _open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, label.c_str());
			ImGui::PopStyleVar();
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("...", ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}
			
			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (typeid(T) != typeid(TransformComponent) && typeid(T) != typeid(IDComponent))
				{
					if (ImGui::MenuItem("Remove component"))
						removeComponent = true;
				}

				
					
				//TODO: copy component for use in another entt

				ImGui::EndPopup();
			}

			if (_open)
			{
				function(component);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			std::strncpy(buffer, tag.c_str(), sizeof(buffer));
			if(ImGui::InputText("##Entity Tag", buffer, sizeof(buffer))) 
			{
				tag = std::string(buffer);
			}
		}
		auto uuid = entity.GetUUID();

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component")) //TODO: hide button if SceneState != edit
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent"))
		{
			DisplayAddComponentEntry<SpriteRendererComponent>("Sprite Renderer");
			DisplayAddComponentEntry<CircleRendererComponent>("Circle Renderer");
			DisplayAddComponentEntry<NativeScriptComponent>("Native Script");
			DisplayAddComponentEntry<CameraComponent>("Camera");
			DisplayAddComponentEntry<LinkPointsComponent>("Link Points");
			DisplayAddComponentEntry<RigidBody2DComponent>("2D Rigid Body");
			DisplayAddComponentEntry<RigidBodyComponent>("Rigid Body");
			DisplayAddComponentEntry<ForceGeneratorComponent>("Force Generator");
			DisplayAddComponentEntry<BoxCollider2DComponent>("2D Box Collider");
			DisplayAddComponentEntry<CircleCollider2DComponent>("2D Circle Collider");
			ImGui::EndPopup();
		}

		DrawComponent<IDComponent>("UUID", entity, [](auto& component)
		{
			ImGui::Separator();
			ImGui::Text("ID: %I64u", component.ID);
			ImGui::SameLine();
			if (ImGui::Button("Copy"))
				ImGui::SetClipboardText(std::to_string(component.ID).c_str());
			ImGui::Separator();
		});

		DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
		{
			DrawVec3Control("Translation", component.Translation, 0.1f);
			glm::vec3 rotation = glm::degrees(component.Rotation);
			DrawVec3Control("Rotation", rotation, 0.25f);
			component.Rotation = glm::radians(rotation);
			DrawVec3Control("Scale", component.Scale, 0.1f, 1.0f);
		});

		ImGui::PopItemWidth();

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
		{
			ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));

			ImGui::Button("Texture", ImVec2(100.f, 0.f)); //button can be used to clear
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path texturePath = std::filesystem::path(g_AssetPath) / path;
					component.Texture = Texture2D::Upload(texturePath.string());
				}
			}

			ImGui::DragFloat("Tiling Factor", &component.TileFactor, 0.1f, 0.0f, 100.f);
		});

		DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](auto& component)
		{
			ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
			ImGui::DragFloat("Thickness", &component.Thickness, 0.025f, 0.0f, 1.0f);
			ImGui::DragFloat("Fade", &component.Fade, 0.00025f, 0.0f, 1.0f);
		});

		DrawComponent<NativeScriptComponent>("Native Script", entity, [](auto& component)
		{
			auto currentClassType = component.Type;

			if (ImGui::BeginCombo("Script Class", Utils::ScriptTypeToString(currentClassType)))
			{
				bool isSelected = currentClassType == ScriptType::None;
				if (ImGui::Selectable("None", isSelected))
				{
					currentClassType = ScriptType::None;
					component.Type = ScriptType::None;
					if (component.DestroyScript)
					{
						component.DestroyScript(&component);
						component.InstantiateScript = nullptr;
					}
				}

				isSelected = currentClassType == ScriptType::CameraController;
				if (ImGui::Selectable("Camera Controller", isSelected))
				{
					currentClassType = ScriptType::CameraController;
					component.Type = ScriptType::CameraController;
					component.Bind<CameraController>();
				}

				isSelected = currentClassType == ScriptType::Test;
				if (ImGui::Selectable("Test", isSelected))
				{
					currentClassType = ScriptType::Test;
					component.Type = ScriptType::Test;
					component.Bind<Test>();
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();


				ImGui::EndCombo();
			}
		});

		DrawComponent<CameraComponent>("Camera", entity, [](auto& component)
		{
			auto& camera = component.Camera;

			ImGui::Checkbox("Primary", &component.Primary);

			const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
			const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
			if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
			{
				for (int i = 0; i < 2; i++)
				{
					bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
					if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
					{
						currentProjectionTypeString = projectionTypeStrings[i];
						camera.SetProjectionType((SceneCamera::ProjectionType)i);
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				float perspectiveVertSize = camera.GetPerspectiveVerticalFOV();
				if (ImGui::DragFloat("Vertical FOV", &perspectiveVertSize))
					camera.SetPerspectiveVerticalFOV(perspectiveVertSize);

				float perspectiveHorzSize = camera.GetPerspectiveHorizontalFOV();
				if (ImGui::DragFloat("Horizontal FOV", &perspectiveHorzSize))
					camera.SetPerspectiveHorizontalFOV(perspectiveHorzSize);

				float perspectiveNear = camera.GetPerspectiveNearClip();
				if (ImGui::DragFloat("Near Clip", &perspectiveNear))
					camera.SetPerspectiveNearClip(perspectiveNear);

				float perspectiveFar = camera.GetPerspectiveFarClip();
				if (ImGui::DragFloat("Far Clip", &perspectiveFar))
					camera.SetPerspectiveFarClip(perspectiveFar);
			}

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
			{
				float orthoSize = camera.GetOrthographicSize();
				if (ImGui::DragFloat("Size", &orthoSize))
					camera.SetOrthographicSize(orthoSize);

				float orthoNear = camera.GetOrthographicNearClip();
				if (ImGui::DragFloat("Near Clip", &orthoNear))
					camera.SetOrthographicNearClip(orthoNear);

				float orthoFar = camera.GetOrthographicFarClip();
				if (ImGui::DragFloat("Far Clip", &orthoFar))
					camera.SetOrthographicFarClip(orthoFar);

				ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
			}
		});
		
		DrawComponent<LinkPointsComponent>("Link Points", entity, [&](auto& component)
		{
			static double xOffset = 0.0;
			static double yOffset = 0.0;
			ImGui::InputDouble("X Offset", &xOffset);
			ImGui::InputDouble("Y Offset", &yOffset);

			if (ImGui::Button("Add", ImVec2(40.f, 0.f)))
			{
				component.Count++;
				m_Context->AddLinkPoint(entity.GetUUID(), {xOffset, yOffset});
				xOffset = yOffset = 0.0;
			}

			ImGui::Separator();

			LinkPointMapIterator iteratorToDelete = LinkPointMapIterator();
			bool deletePoint = false;
			int imGuiID = 0;
			auto range = m_Context->GetLinkPoints(entity.GetUUID());
			for (auto it = range.first; it != range.second; it++, imGuiID++)
			{
				ImGui::PushID(imGuiID);
				DrawVec2Control("Offset", it->second, 0.01);
				ImGui::SameLine();
				
				float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.f;
				ImVec2 buttonSize = { lineHeight + 3.f, lineHeight };
				if (ImGui::Button("x", buttonSize))
				{
					deletePoint = true;
					iteratorToDelete = it;
					component.Count--;
				}
				ImGui::PopID();
			}

			if (deletePoint)
				m_Context->RemoveLinkPoint(iteratorToDelete);
		});

		DrawComponent<RigidBody2DComponent>("Rigidbody 2D", entity, [](auto& component)
		{
			const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
			const char* currentBodyTypeString = bodyTypeStrings[(int)component.Type];
			if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
			{
				for (int i = 0; i < 3; i++)
				{
					bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
					if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
					{
						currentBodyTypeString = bodyTypeStrings[i];
						component.Type = (RigidBody2DComponent::BodyType)i;
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
		});

		DrawComponent<RigidBodyComponent>("Rigid Body", entity, [](auto& component)
		{
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.f;
			ImVec2 buttonSize = { lineHeight + 3.f, lineHeight };

			if (ImGui::Button("X", buttonSize))
				component.Density = 1.0;

			ImGui::SameLine();
			ImGui::DragFloat("Density", &component.Density, 1.0f, 0.001f, 100000.0f);
			ImGui::Checkbox("Fixed Body", &component.Fixed);
		});

		DrawComponent<ForceGeneratorComponent>("Force Generator", entity, [](auto& component)
		{
			const char* genTypeStrings[] = { "Gravity", "Spring", "Gravitational Accelerator" };
			const char* currentGenTypeString = genTypeStrings[(int)component.Type];
			if (ImGui::BeginCombo("Generator Type", currentGenTypeString))
			{
				for (int i = 0; i < 3; i++)
				{
					bool isSelected = currentGenTypeString == genTypeStrings[i];
					if (ImGui::Selectable(genTypeStrings[i], isSelected))
					{
						currentGenTypeString = genTypeStrings[i];
						component.Type = (ForceGeneratorComponent::GeneratorType)i; 
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			if (component.Type == ForceGeneratorComponent::GeneratorType::Gravity)
			{
				float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.f;
				ImVec2 buttonSize = { lineHeight + 3.f, lineHeight };
				if (ImGui::Button("X", buttonSize))
					component.LocalGravity = { 0.0f, -9.81f };

				ImGui::SameLine();
				ImGui::DragFloat2("Local Gravity", glm::value_ptr(component.LocalGravity), 0.100f, -60.0f, 60.0f);
			}

			if (component.Type == ForceGeneratorComponent::GeneratorType::GravitationalAccelerator)
			{
				ImGui::Checkbox("Repulsive Force", &component.RepulsiveForce);
			}
		});

		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component)
		{
			ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
			ImGui::DragFloat2("Size", glm::value_ptr(component.Size));
			ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f); 
			ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f); 
			ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
		});

		DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [](auto& component)
		{
			ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
			ImGui::DragFloat("Radius", &component.Radius);
			ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
		});

	}

	void SceneHierarchyPanel::DeleteDeffered()
	{
		for (auto entity : m_DefferedEntities)
		{
			HZ_CORE_TRACE("DELETING: {0}", entity.GetUUID());
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
		}
	}

	template<typename T>
	void SceneHierarchyPanel::DisplayAddComponentEntry(const std::string& entryName)
	{
		if (!m_SelectionContext.HasComponent<T>())
		{
			if (ImGui::MenuItem(entryName.c_str()))
			{
				m_SelectionContext.AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}
}
