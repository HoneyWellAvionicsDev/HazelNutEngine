#include "hzpch.h"
#include "EulersMethodIntegrator.h"

namespace Enyoo
{
    void EulersMethodIntegrator::Start(SystemState& initalState, double dt)
    {
        m_dt = dt;
    }
    
    bool EulersMethodIntegrator::Step(SystemState& state)
    {
        state.dt = m_dt;
        return true;
    }
    
    void EulersMethodIntegrator::Integrate(SystemState& state)
    {
        for (uint32_t i = 0; i < state.RigidBodyCount; i++)
        {
            state.Position[i] += state.Velocity[i] * m_dt;
            state.Theta[i] += state.AngularVelocity[i] * m_dt;

            state.Velocity[i] += state.Acceleration[i] * m_dt;
            state.AngularVelocity[i] += state.AngularAcceleration[i] * m_dt;
        }
    }
}
