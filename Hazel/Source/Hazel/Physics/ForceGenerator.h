#pragma once

#include "SystemState.h"

namespace Enyoo
{
	class ForceGenerator
	{
	public:
		ForceGenerator() = default;

		void ApplyForce(SystemState& state);

		void SetIndex(size_t index) { m_Index = index; }
		size_t GetIndex() const { return m_Index; }
	private:
		size_t m_Index;
	};
}