#pragma once

#include "Scene.h"

#include <vector>

namespace Hazel
{
	class DynamicSystemAssembler
	{
	public:
		DynamicSystemAssembler() = default;

		bool ProcessEntity(Entity entity);

		bool GenerateAdjacentBodies();
	private:
		
	};
}