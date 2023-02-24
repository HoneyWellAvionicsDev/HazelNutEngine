#include "hzpch.h"
#include "Motor.h"


namespace Enyoo
{
	ConstantSpeedMotor::ConstantSpeedMotor()
		: m_MaxTorque(500.0), m_AngularVelocity(1.0), m_Ks(1.0), m_Kd(1.0), m_BaseBody(nullptr), m_TargetBody(nullptr)
	{
	}
	
	void ConstantSpeedMotor::ApplyForce(SystemState& state)
	{
		double omega;
		double alpha;

		if (m_BaseBody->Index == -1)
		{
			omega = alpha = 0;
		}
		else
		{
			omega = state.AngularVelocity[m_BaseBody->Index];
			alpha = state.AngularAcceleration[m_BaseBody->Index];
		}

		const double rel_omega = state.AngularVelocity[m_TargetBody->Index] - omega;
		const double rel_alpha = state.AngularAcceleration[m_TargetBody->Index] - alpha;
		const double delta = m_AngularVelocity - rel_omega;

		const double torque = delta * m_Ks;
		const double dampingTorque = -rel_alpha * m_Kd;
		const double totalTorque = torque + dampingTorque;
		const double limitedTorque = std::fmin(m_MaxTorque, std::fmax(-m_MaxTorque, totalTorque));

		if (m_BaseBody->Index != -1)
			state.Torque[m_BaseBody->Index] -= limitedTorque;

		state.Torque[m_TargetBody->Index] += limitedTorque;
	}
}
