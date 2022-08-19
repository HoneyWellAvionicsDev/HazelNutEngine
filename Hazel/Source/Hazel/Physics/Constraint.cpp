#include "hzpch.h"
#include "Constraint.h"

namespace Enyoo
{
	Constraint::Constraint(uint32_t constraintCount, uint32_t bodyCount)
		:m_ConstraintCount(constraintCount)
	{
		m_Bodies.reserve(bodyCount);
		m_Index = -1;

		//set stored forces to zero
	}

	void Constraint::Calculate(SystemState* state)
	{
	}
}
