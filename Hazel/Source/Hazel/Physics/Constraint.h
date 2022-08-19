#pragma once

#include "Hazel/Math/Matrix.h"

#include "RigidBody.h"
#include "SystemState.h"

namespace Enyoo
{
	struct ConstraintOutput
	{

	};

	class Constraint
	{
	public:
		Constraint(uint32_t ConstraintCount, uint32_t BodyCount);
		virtual ~Constraint();

		virtual void Calculate(/*pointer to output,*/ SystemState* state); //make this pure virtual when this becomes a base class

		void SetIndex(size_t index) { m_Index = index; }
		size_t GetBodyCount() const { return m_Bodies.size(); }
		size_t GetIndex() const { return m_Index; }
	protected:
		size_t m_Index;
		uint32_t m_ConstraintCount;
		std::vector<RigidBody*> m_Bodies;
	};
}
