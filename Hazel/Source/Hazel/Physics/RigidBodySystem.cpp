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
        PopulateMassMatrices(m_MatricesData.Mass, m_MatricesData.W);

        for (int i = 0; i < steps; i++)
        {
            m_Solver.Start(m_State, dt / steps);

            while (true)
            {
                const bool done = m_Solver.Step(m_State);
                UpdateForces();
                ResolveConstraints();
                m_Solver.Integrate(m_State);

                if (done) break;
            }

            m_Solver.End();
        }

        for (uint32_t i = 0; i < GetRigidBodyCount(); i++) //assign updated state to rigid bodies
        {
            m_RigidBodies[i]->Velocity = m_State.Velocity[i];
            m_RigidBodies[i]->Position = m_State.Position[i];

            m_RigidBodies[i]->AngularVelocity = m_State.AngularVelocity[i];
            m_RigidBodies[i]->Theta = m_State.Theta[i];
        }

        for (Constraint* constraint : m_Constraints) //assign updated state to constraints
        {
            for (size_t i = 0, c_i; i < constraint->GetConstraintCount(); i++, c_i++)
            {
                for (size_t j = 0; j < constraint->GetBodyCount(); j++)
                {
                    constraint->ConstraintForceX[i][j] = m_State.ConstraintForce[c_i].x;
                    constraint->ConstraintForceY[i][j] = m_State.ConstraintForce[c_i].y;
                    constraint->ConstraintTorque[i][j] = m_State.ConstraintTorque[c_i];
                }
            }
        }
    }

    void RigidBodySystem::AddRigidBody(RigidBody* body)
    {
        m_RigidBodies.push_back(body);
        body->Index = m_RigidBodies.size() - 1;
    }

    void RigidBodySystem::RemoveRigidBody(RigidBody* body)
    {
    }

    void RigidBodySystem::AddForceGen(ForceGenerator* forceGen)
    {
        m_ForceGenerators.push_back(forceGen);
        forceGen->SetIndex(m_ForceGenerators.size() - 1);
    }

    void RigidBodySystem::AddConstraint(Constraint* constraint)
    {
        m_Constraints.push_back(constraint);
        constraint->SetIndex(m_Constraints.size() - 1);
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

            m_State.Mass[i] = m_RigidBodies[i]->Mass;
        }
    }

    void RigidBodySystem::PopulateMassMatrices(Matrix& Mass, Matrix& massInverse)
    {
        const uint32_t n = GetRigidBodyCount();

        Mass.Initialize(n * 3, 1);
        massInverse.Initialize(n * 3, 1);

        for (uint32_t i = 0; i < n; i++)
        {
            Mass[i * 3 + 0][0] = m_RigidBodies[i]->Mass;
            Mass[i * 3 + 1][0] = m_RigidBodies[i]->Mass;
            Mass[i * 3 + 2][0] = m_RigidBodies[i]->MomentInertia;
     
            massInverse[i * 3 + 0][0] = m_RigidBodies[i]->Mass;
            massInverse[i * 3 + 1][0] = m_RigidBodies[i]->Mass;
            massInverse[i * 3 + 2][0] = m_RigidBodies[i]->MomentInertia;
        }
    }

    void RigidBodySystem::UpdateForces()
    {
        //zero out forces
        for (uint32_t i = 0; i < GetRigidBodyCount(); i++)
        {
            m_State.Force[i] = glm::dvec2{ 0.0 };
            m_State.Acceleration[i] = glm::dvec2{ 0.0 };
            m_State.Torque[i] = 0.0;
        }
        //loop through force generators and apply their force to the state
        for (ForceGenerator* forceGen : m_ForceGenerators)
            forceGen->ApplyForce(m_State);
    }

    void RigidBodySystem::ResolveConstraints()
    {

    }
}