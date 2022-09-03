#pragma once

#include "SystemState.h"

namespace Enyoo
{
	class ForceGenerator
	{
	public:
		ForceGenerator() = default;
		virtual ~ForceGenerator() = default;

		virtual void ApplyForce(SystemState& state) = 0;

		void SetIndex(size_t index) { m_Index = index; }
		size_t GetIndex() const { return m_Index; }
	protected:
		size_t m_Index;
	};
}