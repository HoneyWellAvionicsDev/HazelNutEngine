#pragma once

#include "SystemState.h"
#include "RigidBody.h"

namespace Enyoo
{
	class ForceGenerator
	{
	public:
		ForceGenerator() = default;
		virtual ~ForceGenerator() = default;

		virtual void ApplyForce(SystemState& state) = 0;

		void SetIndex(size_t index) { m_Index = index; }
		void SetActive(bool active) { m_Actice = active; }
		size_t GetIndex() const { return m_Index; }
		bool IsActive() const { return m_Actice; }
	protected:
		size_t m_Index;
		bool m_Actice = true;
	};
}