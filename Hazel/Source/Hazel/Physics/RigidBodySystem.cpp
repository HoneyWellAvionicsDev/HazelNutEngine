#include <hzpch.h>

#include "RigidBodySystem.h"

namespace Enyoo
{
    void RigidBodySystem::Init()
    {

    }

    void RigidBodySystem::Step(double dt, int steps)
    {
        PopulateSystemState();

        for (int i = 0; i < steps; i++)
        {
            m_Solver.Start(m_State, dt / steps);

            while (true)
            {
                const bool done = m_Solver.Step(m_State);
                UpdateForces();
                //UpdateConstraints();
                m_Solver.Integrate(m_State);

                if (done) break;
            }

            m_Solver.End();
        }

        for (uint32_t i = 0; i < GetRigidBodyCount(); i++)
        {
            m_RigidBodies[i]->Velocity = m_State.Velocity[i];
            m_RigidBodies[i]->Position = m_State.Position[i];

            m_RigidBodies[i]->AngularVelocity = m_State.AngularVelocity[i];
            m_RigidBodies[i]->Theta = m_State.Theta[i];
        }
    }

    void RigidBodySystem::AddRigidBody(RigidBody* body)
    {
        m_RigidBodies.push_back(body);
        body->Index = (int)m_RigidBodies.size() - 1;
    }

    void RigidBodySystem::RemoveRigidBody(RigidBody* body)
    {
    }

    void RigidBodySystem::AddForceGen(ForceGenerator* forceGen)
    {
        m_ForceGenerators.push_back(forceGen);
        forceGen->m_Index = (int)m_ForceGenerators.size() - 1;
    }

    void RigidBodySystem::PopulateSystemState()
    {
        m_State.Resize(GetRigidBodyCount());

        for (size_t i = 0; i < GetRigidBodyCount(); i++)
        {
            m_State.Acceleration[i] = glm::dvec2{ 0.0 };

            m_State.Velocity[i] = m_RigidBodies[i]->Velocity;
            m_State.Position[i] = m_RigidBodies[i]->Position;

            m_State.AngularAcceleration[i] = 0;
            m_State.AngularVelocity[i] = m_RigidBodies[i]->AngularVelocity;
            m_State.Theta[i] = m_RigidBodies[i]->Theta;
        }
    }

    void RigidBodySystem::UpdateForces()
    {
        //zero out forces
        for (uint32_t i = 0; i < GetRigidBodyCount(); i++)
        {
            m_State.Force[i] = glm::dvec2{ 0.0 };
            m_State.Torque[i] = 0.0;
        }
        //loop through force generators and apply their force to the state
        for (ForceGenerator* forceGen : m_ForceGenerators)
            forceGen->ApplyForce(m_State);
    }
}