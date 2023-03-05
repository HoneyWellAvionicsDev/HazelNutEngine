#pragma once

#include "Hazel/Core/TimeStep.h"
#include "Hazel/Core/UUID.h"
#include "Hazel/Physics/RigidBodySystem.h"
#include "Hazel/Renderer/EditorCamera.h"

#include "entt.hpp"

class b2World;


namespace Jbonk
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
		static Ref<Scene> CopyExclusive(Ref<Scene> other, Entity exclusion);

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		Entity DuplicateEntity(Entity entity);
		Entity GetEntity(UUID uuid);
		Entity GetPrimaryCameraEntity();
		const entt::registry& GetRegistry() const { return m_Registry; }
		bool Valid(Entity entity) const;

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
		void SetPhysicsDT(double dt) { m_PhysicsDeltaT = dt; }
		void SetPhysicsSteps(uint16_t steps) { m_PhysicsSteps = steps; }
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
		double m_PhysicsDeltaT = 0.01667;
		uint16_t m_PhysicsSteps = 1;

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


