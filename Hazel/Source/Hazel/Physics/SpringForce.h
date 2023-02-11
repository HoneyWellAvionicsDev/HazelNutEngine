#pragma once

#include "ForceGenerator.h"

namespace Enyoo
{
	class Spring : public ForceGenerator
	{
	public:
		Spring();

		virtual void ApplyForce(SystemState& systemState) override;
	};
}