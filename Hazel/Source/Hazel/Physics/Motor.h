#pragma once

#include "ForceGenerator.h"

namespace Enyoo
{
    class ConstantSpeedMotor : public ForceGenerator
    {
    public:
        ConstantSpeedMotor();
        virtual ~ConstantSpeedMotor() = default;

        virtual void ApplyForce(SystemState& state) override;
        void SetBaseBody(RigidBody* body) { m_BaseBody = body; }
        void SetTargetBody(RigidBody* body) { m_TargetBody = body; }
        void SetMaxTorque(double torque) { m_MaxTorque = torque; }
        void SetSpeed(double speed) { m_AngularVelocity = speed; }
    private: 
        double m_MaxTorque;
        double m_AngularVelocity;

        double m_Ks;
        double m_Kd;

        RigidBody* m_BaseBody;
        RigidBody* m_TargetBody;
    };
}

