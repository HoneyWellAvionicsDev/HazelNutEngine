#pragma once

#include "Hazel/Core/UUID.h"
#include "Components.h"
#include "Scene.h"
#include "entt.hpp"

#include <glm/glm.hpp>


namespace Jbonk
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			JB_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}                                                         //dont unpack our args here, instead forward them to entt

		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args)
		{
			T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T>
		T& GetComponent()
		{
			JB_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
		}

		template<typename T>
		void RemoveComponent()
		{
			JB_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			m_Scene->OnComponentRemoved<T>(*this, GetComponent<T>());
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		UUID GetUUID() { return GetComponent<IDComponent>().ID; }
		Scene* GetScene() const { return m_Scene; }
		entt::entity GetHandle() const { return m_EntityHandle; }
		const std::string& GetName() { return GetComponent<TagComponent>().Tag; }
		glm::mat4 GetTransform() { return GetComponent<TransformComponent>().GetTransform(); } //TODO: this should return a reference

		operator bool() const { return m_EntityHandle != entt::null; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }
		operator uint64_t() const { return (uint64_t)(uint32_t)m_EntityHandle; }
		operator entt::entity() const { return m_EntityHandle; }

		bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }
		bool operator!=(const Entity& other) const { return !(*this == other); }
		bool operator<(const Entity& other) const { return m_EntityHandle < other.m_EntityHandle; }

	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr; 
	};
}

namespace std
{
	template<typename T> struct hash;

	template<>
	struct hash<Jbonk::Entity>
	{
		size_t operator()(const Jbonk::Entity& entity) const
		{
			return static_cast<uint32_t>(entity);
		}
	};
}