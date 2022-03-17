#pragma once

#include "entt.hpp"
#include "Hazel/Core/TimeStep.h"

namespace Hazel
{
	class Scene
	{
	public:
		Scene();
		~Scene();

		entt::entity CreateEntity();

		//remove later
		entt::registry& Reg() { return m_Registry; }
		void OnUpdate(Timestep ts);
	private:
		entt::registry m_Registry;

	};
}

