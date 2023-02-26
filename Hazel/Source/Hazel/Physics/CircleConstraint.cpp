#include "hzpch.h"

#include "CircleConstraint.h"

namespace Enyoo
{
	CircleConstraint::CircleConstraint()
		: Constraint(2, 2), m_LocalPosition(glm::dvec2(0.0)), m_Dx(1.0), m_Dy(0.0), m_Radius(0.0),
		m_Ks(10.0), m_Kd(0.0)
	{
        J1.Initialize(m_ConstraintCount, 3);
        J2.Initialize(m_ConstraintCount, 3);
        J1dot.Initialize(m_ConstraintCount, 3);
        J2dot.Initialize(m_ConstraintCount, 3);
	}

	void CircleConstraint::Calculate(ConstraintOutput& output, SystemState& state)
	{
		const size_t baseBody = m_Bodies.front()->Index;
		const size_t rollingBody = m_Bodies.back()->Index;

		const double q1 = state.Position[baseBody].x;
		const double q2 = state.Position[baseBody].y;
		const double q3 = state.Theta[baseBody];

		const double q4 = state.Position[rollingBody].x;
		const double q5 = state.Position[rollingBody].y;
		const double q6 = state.Theta[rollingBody];

		const double q1_dot = state.Velocity[baseBody].x;
		const double q2_dot = state.Velocity[baseBody].y;
		const double q3_dot = state.AngularVelocity[baseBody];

		const double q4_dot = state.Velocity[rollingBody].x;
		const double q5_dot = state.Velocity[rollingBody].y;

		const double cos_q3 = glm::cos(q3);
		const double sin_q3 = glm::sin(q3);

        const double origin_x = q1 + cos_q3 * m_LocalPosition.x - sin_q3 * m_LocalPosition.y;
        const double origin_y = q2 + sin_q3 * m_LocalPosition.x + cos_q3 * m_LocalPosition.y;
        const double dx = cos_q3 * m_Dx - sin_q3 * m_Dy;
        const double dy = sin_q3 * m_Dx + cos_q3 * m_Dy;

        const double dx_dot = -sin_q3 * q3_dot * m_Dx - cos_q3 * q3_dot * m_Dy;
        const double dy_dot = cos_q3 * q3_dot * m_Dx - sin_q3 * q3_dot * m_Dy;

        const double perp_x = -dy;
        const double perp_y = dx;

        const double delta_x = q4 - origin_x;
        const double delta_y = q5 - origin_y;

        const double delta_x_dot =
            q4_dot - (q1_dot - sin_q3 * q3_dot * m_LocalPosition.x - cos_q3 * q3_dot * m_LocalPosition.y);
        const double delta_y_dot =
            q5_dot - (q2_dot + cos_q3 * q3_dot * m_LocalPosition.x - sin_q3 * q3_dot * m_LocalPosition.y);

        const double s = delta_x * dx + delta_y * dy;

        const double C0 = -q6 - s * m_Radius;
        const double C1 = m_Radius - (perp_x * delta_x + perp_y * delta_y);

        const double d_origin_x_dq3 = -sin_q3 * m_LocalPosition.x - cos_q3 * m_LocalPosition.y;
        const double d_origin_y_dq3 = cos_q3 * m_LocalPosition.x - sin_q3 * m_LocalPosition.y;

        const double d_delta_x_dq1 = -1;
        const double d_delta_x_dq3 = -d_origin_x_dq3;
        const double d_delta_x_dq4 = 1;

        const double d_delta_y_dq2 = -1;
        const double d_delta_y_dq3 = -d_origin_y_dq3;
        const double d_delta_y_dq5 = 1;

        const double d_dx_dq3 = -dy;
        const double d_dy_dq3 = dx;

        const double d_dx_dq3_dot = -cos_q3 * q3_dot * m_Dx + sin_q3 * q3_dot * m_Dy;
        const double d_dy_dq3_dot = -sin_q3 * q3_dot * m_Dx - cos_q3 * q3_dot * m_Dy;
        const double d_delta_x_dq3_dot =
            cos_q3 * q3_dot * m_LocalPosition.x - sin_q3 * q3_dot * m_LocalPosition.y;
        const double d_delta_y_dq3_dot =
            sin_q3 * q3_dot * m_LocalPosition.x + cos_q3 * q3_dot * m_LocalPosition.y;

        const double ds_dq1 = d_delta_x_dq1 * dx;
        const double ds_dq2 = d_delta_y_dq2 * dy;
        const double ds_dq3 =
            (d_delta_x_dq3 * dx + delta_x * d_dx_dq3) +
            (d_delta_y_dq3 * dy + delta_y * d_dy_dq3);

        const double ds_dq1_dot =
            d_delta_x_dq1 * dx_dot;
        const double ds_dq2_dot =
            d_delta_y_dq2 * dy_dot;
        const double ds_dq3_dot =
            (d_delta_x_dq3_dot * dx + d_delta_x_dq3 * dx_dot) +
            (delta_x_dot * d_dx_dq3 + delta_x * d_dx_dq3_dot) +
            (d_delta_y_dq3_dot * dy + d_delta_y_dq3 * dy_dot) +
            (delta_y_dot * d_dy_dq3 + delta_y * d_dy_dq3_dot);

        output.ks.Resize(m_ConstraintCount, 1);
        output.kd.Resize(m_ConstraintCount, 1);
        output.C.Resize(m_ConstraintCount, 1);

        J1[0][0] = -ds_dq1 * m_Radius;
        J1[0][1] = -ds_dq2 * m_Radius;
        J1[0][2] = -ds_dq3 * m_Radius;

        J1[1][0] = dy * d_delta_x_dq1;
        J1[1][1] = -dx * d_delta_y_dq2;
        J1[1][2] =
            (d_dy_dq3 * delta_x + dy * d_delta_x_dq3) -
            (d_dx_dq3 * delta_y + dx * d_delta_y_dq3);

        J2[0][0] = -1 * dx * m_Radius;
        J2[0][1] = -1 * dy * m_Radius;
        J2[0][2] = -1;

        J2[1][0] = dy * d_delta_x_dq4;
        J2[1][1] = -dx * d_delta_y_dq5;
        J2[1][2] = 0;

        J1dot[0][0] = -ds_dq1_dot * m_Radius;
        J1dot[0][1] = -ds_dq2_dot * m_Radius;
        J1dot[0][2] = -ds_dq3_dot * m_Radius;

        J1dot[1][0] = dy_dot * d_delta_x_dq1;
        J1dot[1][1] = -dx_dot * d_delta_y_dq2;
        J1dot[1][2] =
            (d_dy_dq3_dot * delta_x + d_dy_dq3 * delta_x_dot) +
            (dy_dot * d_delta_x_dq3 + dy * d_delta_x_dq3_dot) -
            (d_dx_dq3_dot * delta_y + d_dx_dq3 * delta_y_dot) -
            (dx_dot * d_delta_y_dq3 + dx * d_delta_y_dq3_dot);

        J2dot[0][0] = -1 * dx_dot * m_Radius;
        J2dot[0][1] = -1 * dy_dot * m_Radius;
        J2dot[0][2] = 0;

        J2dot[1][0] = dy_dot * d_delta_x_dq4;
        J2dot[1][1] = -dx_dot * d_delta_y_dq5;
        J2dot[1][2] = 0;

        output.ks[0][0] = 0;
        output.kd[0][0] = 0;

        output.ks[1][0] = m_Ks;
        output.kd[1][0] = m_Kd;

        output.C[0][0] = C0;
        output.C[1][0] = C1;

        output.J.clear();
        output.Jdot.clear();
        output.J.push_back(J1);
        output.J.push_back(J2);
        output.Jdot.push_back(J1dot);
        output.Jdot.push_back(J2dot);
	}
}