#pragma once

#include "Hazel/Core/TimeStep.h"
#include "Hazel/Core/UUID.h"
#include "Hazel/Physics/RigidBodySystem.h"
#include "Hazel/Renderer/EditorCamera.h"

#include "entt.hpp"

class b2World;

namespace Hazel
{
	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		static Ref<Scene> Copy(Ref<Scene> other);

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnUpdateRuntime(Timestep ts);
		void OnUpdateRuntime(Timestep ts, EditorCamera& camera);
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		void OnUpdateSimulation(Timestep ts, EditorCamera& camera);

		void OnSimulationStart();
		void OnSimulationStop();

		void OnViewportResize(uint32_t width, uint32_t height);

		void DuplicateEntity(Entity entity);

		Entity GetPrimaryCameraEntity();

		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}

		void SetVelocityIterations(uint16_t iter) { m_VelocityIterations = iter; }
		void SetPositionIterations(uint16_t iter) { m_PositionIterations = iter; }
		void SetLevelGravity(glm::vec2 localAcceleration) { m_LocalGravity = localAcceleration; }
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		void OnPhysics2DStart();
		void OnPhysics2DStop();
		void Update2DPhysics(Timestep ts);
		void OnPhysicsStart();
		void OnPhysicsStop();
		void UpdatePhysics(Timestep ts);
		void UpdateScripts(Timestep ts);

		void RenderSceneEntities();

		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;

		uint16_t m_VelocityIterations = 6;
		uint16_t m_PositionIterations = 2;
		glm::vec2 m_LocalGravity{ 0.0f };
		b2World* m_PhysicsWorld = nullptr;
		Enyoo::RigidBodySystem* m_NewBodySystem = nullptr;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}


