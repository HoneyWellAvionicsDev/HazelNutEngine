#include "hzpch.h"
#include "Constraint.h"

namespace Enyoo
{
	Constraint::Constraint(uint32_t constraintCount, uint32_t bodyCount)
		: m_ConstraintCount(constraintCount)
	{
		m_Bodies.reserve(bodyCount);
		m_Index = -1;

		memset(&m_Bodies[0], 0, sizeof(int) * bodyCount);
		
		ConstraintForceX.Initialize(constraintCount,bodyCount);
		ConstraintForceY.Initialize(constraintCount, bodyCount);
		ConstraintTorque.Initialize(constraintCount, bodyCount);

		for (uint32_t i = 0; i < constraintCount; i++)
		{
			for (uint32_t j = 0; j < bodyCount; j++)
			{
				ConstraintForceX[i][j] = ConstraintForceY[i][j] = ConstraintTorque[i][j] = 0.0;
			}
		}
	}
}
