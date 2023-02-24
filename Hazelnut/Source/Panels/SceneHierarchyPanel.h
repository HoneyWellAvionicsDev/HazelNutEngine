#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Core/Log.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Entity.h"

namespace Hazel
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void SetContext(const Ref<Scene>& scene);
		void SetSelectionContext(const Entity& ent) { m_SelectionContext = ent; }
		
		void OnImGuiRender();

		Entity GetSelectedEntity() const { return m_SelectionContext; }
		bool DependencyCheck() const { return m_SetDependency; }
		void CheckTerminate() { m_SetDependency = false; }
	private:
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
		template<typename T>
		void DisplayAddComponentEntry(const std::string& entryName);
	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
		bool m_SetDependency = false;
	};
}

