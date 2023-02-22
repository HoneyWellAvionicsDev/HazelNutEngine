#pragma once

#include "Hazel/Core/TimeStep.h"
#include "Hazel/Core/UUID.h"
#include "Hazel/Physics/RigidBodySystem.h"
#include "Hazel/Renderer/EditorCamera.h"

#include "entt.hpp"

class b2World;


namespace Hazel
{
	using LinkPointMap = std::unordered_multimap<UUID, glm::vec2>;
	using LinkPointMapIterator = LinkPointMap::iterator;
	using IteratorPair = std::pair<LinkPointMapIterator, LinkPointMapIterator>;

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

		Entity DuplicateEntity(Entity entity);

		Entity GetEntity(UUID uuid);
		Entity GetPrimaryCameraEntity();

		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}

		LinkPointMap GetLinkPointMap() const { return m_EntityLinkPointMap; }
		Ref<Enyoo::RigidBodySystem> GetRigidBodySystem() const { return m_ConstrainedBodySystem; }
		std::filesystem::path GetSceneName() const { return m_SceneName; }

		IteratorPair GetLinkPoints(UUID uuid) { return m_EntityLinkPointMap.equal_range(uuid); }
		void AddLinkPoint(UUID uuid, glm::dvec2 linkPoint) { m_EntityLinkPointMap.emplace(uuid, linkPoint); }
		void RemoveLinkPoints(UUID uuid);
		void RemoveLinkPoint(LinkPointMapIterator iter) { m_EntityLinkPointMap.erase(iter); }

		void SetSceneName(const std::filesystem::path& fileName) { m_SceneName = fileName; }
		void SetVelocityIterations(uint16_t iter) { m_VelocityIterations = iter; }
		void SetPositionIterations(uint16_t iter) { m_PositionIterations = iter; }
		void SetLevelGravity(glm::vec2 localAcceleration) { m_LocalGravity = localAcceleration; }
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
		template<typename T>
		void OnComponentRemoved(Entity entity, T& component);

		void OnPhysics2DStart();
		void OnPhysics2DStop();
		void OnPhysicsStart();
		void OnPhysicsStop();
		void Update2DPhysics(Timestep ts);
		void UpdatePhysics(Timestep ts);
		void UpdateScripts(Timestep ts);

		void RenderSceneEntities();
	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;
		std::filesystem::path m_SceneName;

		uint16_t m_VelocityIterations = 6;
		uint16_t m_PositionIterations = 2;
		glm::vec2 m_LocalGravity{ 0.0f };
		b2World* m_PhysicsWorld = nullptr;

		Ref<Enyoo::RigidBodySystem> m_ConstrainedBodySystem = nullptr;
		std::unordered_multimap<UUID, glm::vec2> m_EntityLinkPointMap;

		friend class Entity;
		friend class EditorLayer;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
		friend class DynamicSystemAssembler;
	};
}


