#include "hzpch.h"
#include "RungeKutta4thIntegrator.h"

namespace Enyoo
{
    void RungeKutta4thIntegrator::Start(SystemState& initalState, double dt)
    {
        ODEIntegrator::Start(initalState, dt);

        m_InitialState = initalState;
        m_Accumulator = initalState;
        m_CurrentStage = Stage::K1;

    }
    
    bool RungeKutta4thIntegrator::Step(SystemState& state)
    {
        switch (m_CurrentStage)
        {
            case Stage::K1:
            {
                state.dt = 0.0;
                break;
            }
            case Stage::K2:
            case Stage::K3:
            {
                for (uint32_t i = 0; i < state.RigidBodyCount; i++)
                {
                    state.Position[i]        = m_InitialState.Position[i] + state.Velocity[i] * m_dt * 0.5;
                    state.Theta[i]           = m_InitialState.Theta[i] + state.AngularVelocity[i] * m_dt * 0.5;
                    
                    state.Velocity[i]        = m_InitialState.Velocity[i] + state.Acceleration[i] * m_dt * 0.5;
                    state.AngularVelocity[i] = m_InitialState.AngularVelocity[i] + state.AngularAcceleration[i] * m_dt * 0.5;
                }

                state.dt = m_dt / 2.0;
                break;
            }
            case Stage::K4:
            {
                for (uint32_t i = 0; i < state.RigidBodyCount; i++)
                {
                    state.Position[i]        = m_InitialState.Position[i] + state.Velocity[i] * m_dt;
                    state.Theta[i]           = m_InitialState.Theta[i] + state.AngularVelocity[i] * m_dt;

                    state.Velocity[i]        = m_InitialState.Velocity[i] + state.Acceleration[i] * m_dt;
                    state.AngularVelocity[i] = m_InitialState.AngularVelocity[i] + state.AngularAcceleration[i] * m_dt;
                }

                state.dt = m_dt;
                break;
            }
            default:
                break;
        }

        m_NextStage = (Stage)((int)m_CurrentStage + 1);

        return m_NextStage == Stage::Done;
    }
    
    void RungeKutta4thIntegrator::Integrate(SystemState& state)
    {
        double slopeWeight;
        switch (m_CurrentStage)
        {
            case Stage::K1: slopeWeight = 1.0; break;
            case Stage::K2: slopeWeight = 2.0; break;
            case Stage::K3: slopeWeight = 2.0; break;
            case Stage::K4: slopeWeight = 1.0; break;
            default: slopeWeight = 0.0; break;
        }

        // y(n + 1) = yn * h/6 * (k1 + 2k2 + 2k3 + k4);
        for (uint32_t i = 0; i < state.RigidBodyCount; i++)
        {
            m_Accumulator.Position[i]        += (m_dt / 6.0) * state.Velocity[i] * slopeWeight;
            m_Accumulator.Theta[i]           += (m_dt / 6.0) * state.AngularVelocity[i] * slopeWeight;

            m_Accumulator.Velocity[i]        += (m_dt / 6.0) * state.Acceleration[i] * slopeWeight;
            m_Accumulator.AngularVelocity[i] += (m_dt / 6.0) * state.AngularAcceleration[i] * slopeWeight;
        }

        if (m_CurrentStage == Stage::K4)
        {
            for (uint32_t i = 0; i < state.RigidBodyCount; i++)
            {
                state.Velocity[i]        = m_Accumulator.Velocity[i];
                state.AngularVelocity[i] = m_Accumulator.AngularVelocity[i];

                state.Position[i]        = m_Accumulator.Position[i];
                state.Theta[i]           = m_Accumulator.Theta[i];
            }
        }

        m_CurrentStage = m_NextStage;
    }
}
