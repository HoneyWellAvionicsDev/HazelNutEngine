#include "hzpch.h"
#include "FixedPositionConstraint.h"

namespace Enyoo
{
	FixedPositionConstraint::FixedPositionConstraint()
		: Constraint(2, 1), m_LocalPosition{ 0.0, 0.0 }, m_WorldPosition{ 0.0, 0.0 }, m_ks(10.0), m_kd(1.0)
	{
	}
	
	FixedPositionConstraint::~FixedPositionConstraint()
	{
	}
	
	void FixedPositionConstraint::Calculate(ConstraintOutput& output, SystemState* state)
	{
		const size_t body = m_Bodies[0]->Index;

        const double q1 = state->Position[body].x;
        const double q2 = state->Position[body].y;
        const double q3 = state->Theta[body];

        const double q3dot = state->AngularVelocity[body];

        const double cos_q3 = glm::cos(q3);
        const double sin_q3 = glm::sin(q3);

        const double current_x = q1 + cos_q3 * m_LocalPosition.x - sin_q3 * m_LocalPosition.x;
        const double current_y = q2 + sin_q3 * m_LocalPosition.y + cos_q3 * m_LocalPosition.y;

        const double dx_dq1 = 1.0;
        const double dx_dq2 = 0.0;
        const double dx_dq3 = -sin_q3 * m_LocalPosition.x - cos_q3 * m_LocalPosition.y;

        const double dy_dq1 = 0.0;
        const double dy_dq2 = 1.0;
        const double dy_dq3 = cos_q3 * m_LocalPosition.x - sin_q3 * m_LocalPosition.y;

        const double C1 = current_x - m_WorldPosition.x;
        const double C2 = current_y - m_WorldPosition.y;

        output.J.Initialize(m_ConstraintCount, 3 * m_Bodies.size());
        output.Jdot.Initialize(m_ConstraintCount, 3 * m_Bodies.size());
        output.ks.Initialize(m_ConstraintCount, 1);
        output.kd.Initialize(m_ConstraintCount, 1);
        output.C.Initialize(m_ConstraintCount, 1);

        output.J[0][0] = dx_dq1;
        output.J[0][1] = dx_dq2;
        output.J[0][2] = dx_dq3;
              
        output.J[1][0] = dy_dq1;
        output.J[1][1] = dy_dq2;
        output.J[1][2] = dy_dq3;
              
        output.Jdot[0][0] = 0;
        output.Jdot[0][1] = 0;
        output.Jdot[0][2] = -cos_q3 * q3dot * m_LocalPosition.x + sin_q3 * q3dot * m_LocalPosition.y;
              
        output.Jdot[1][0] = 0;
        output.Jdot[1][1] = 0;
        output.Jdot[1][2] = -sin_q3 * q3dot * m_LocalPosition.x - cos_q3 * q3dot * m_LocalPosition.y;
              
        output.ks[0][0] = m_ks;
        output.ks[1][0] = m_ks;
              
        output.kd[0][0] = m_kd;
        output.kd[1][0] = m_kd;
              
        output.C[0][0] = C1;
        output.C[1][0] = C2;
      
	}
}
