#include "hzpch.h"
#include "Constraint.h"


namespace Enyoo
{
	Constraint::Constraint(uint32_t constraintCount, uint32_t bodyCount)
		: m_ConstraintCount(constraintCount), m_BodyCount(bodyCount), m_Index(-1)
	{
	}
}
