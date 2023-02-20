#include <hzpch.h>

#include "LinkConstraint.h"

namespace Enyoo
{
	LinkConstraint::LinkConstraint()
		: Constraint(2, 2), m_FirstBodyLocal({0.0, 0.0}), m_SecondBodyLocal({0.0, 0.0}), m_ks(20.0), m_kd(2.0)
	{
		J1.Initialize(m_ConstraintCount, 3);
		J2.Initialize(m_ConstraintCount, 3);
		J1dot.Initialize(m_ConstraintCount, 3);
		J2dot.Initialize(m_ConstraintCount, 3);
	}

	void LinkConstraint::Calculate(ConstraintOutput& output, SystemState& state)
	{
		const size_t focusBody = m_Bodies[0]->Index;
		const size_t targetBody = m_Bodies[1]->Index;

		const double q3 = state.Theta[focusBody];
		const double q6 = state.Theta[targetBody];

		const double q3dot = state.AngularVelocity[focusBody]; 
		const double q6dot = state.AngularVelocity[targetBody];

		const double sinQ3 = glm::sin(q3);
		const double cosQ3 = glm::cos(q3);
							 
		const double sinQ6 = glm::sin(q6);
		const double cosQ6 = glm::cos(q6);

		const double bodyX = state.Position[focusBody].x + cosQ3 * m_FirstBodyLocal.x - sinQ3 * m_FirstBodyLocal.y;
		const double bodyY = state.Position[focusBody].y + sinQ3 * m_FirstBodyLocal.x + cosQ3 * m_FirstBodyLocal.y;

		const double linkedBodyX = state.Position[targetBody].x + cosQ6 * m_SecondBodyLocal.x - sinQ6 * m_SecondBodyLocal.y;
		const double linkedBodyY = state.Position[targetBody].y + sinQ6 * m_SecondBodyLocal.x + cosQ6 * m_SecondBodyLocal.y;

		output.ks.Resize(m_ConstraintCount, 1);
		output.kd.Resize(m_ConstraintCount, 1);
		output.C.Resize(m_ConstraintCount, 1);

		J1[0][0] = 1.0;
		J1[0][1] = 0.0;
		J1[0][2] = -sinQ3 * m_FirstBodyLocal.x - cosQ3 * m_FirstBodyLocal.y;

		J1[1][0] = 0.0;
		J1[1][1] = 1.0;
		J1[1][2] = cosQ3 * m_FirstBodyLocal.x - sinQ3 * m_FirstBodyLocal.y;

		J2[0][0] = -1.0;
		J2[0][1] = 0.0;
		J2[0][2] = sinQ6 * m_SecondBodyLocal.x + cosQ6 * m_SecondBodyLocal.y;

		J2[1][0] = 0.0;
		J2[1][1] = -1.0;
		J2[1][2] = -cosQ6 * m_SecondBodyLocal.x + sinQ6 * m_SecondBodyLocal.y;

		J1dot[0][0] = 0;
		J1dot[0][1] = 0;
		J1dot[0][2] = -cosQ3 * q3dot * m_FirstBodyLocal.x + sinQ3 * q3dot * m_FirstBodyLocal.y;

		J1dot[1][0] = 0;
		J1dot[1][1] = 0;
		J1dot[1][2] = -sinQ3 * q3dot * m_FirstBodyLocal.x - cosQ3 * q3dot * m_FirstBodyLocal.y;

		J2dot[0][0] = 0;
		J2dot[0][1] = 0;
		J2dot[0][2] = cosQ6 * q6dot * m_SecondBodyLocal.x - sinQ6 * q6dot * m_SecondBodyLocal.y;
		
		J2dot[1][0] = 0;
		J2dot[1][1] = 0;
		J2dot[1][2] = sinQ6 * q6dot * m_SecondBodyLocal.x + cosQ6 * q6dot * m_SecondBodyLocal.y;

		output.kd[0][0] = m_kd;
		output.kd[1][0] = m_kd;

		output.ks[0][0] = m_ks;
		output.ks[1][0] = m_ks;

		output.C[0][0] = bodyX - linkedBodyX;
		output.C[1][0] = bodyY - linkedBodyY;

		output.J.clear();
		output.Jdot.clear();
		output.J.push_back(J1);
		output.J.push_back(J2);
		output.Jdot.push_back(J1dot);
		output.Jdot.push_back(J2dot);
	}
}
