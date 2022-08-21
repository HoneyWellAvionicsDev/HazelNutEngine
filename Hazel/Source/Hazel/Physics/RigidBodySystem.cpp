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
                //ResolveConstraints();
                m_Solver.Integrate(m_State);

                if (done) break;
            }

            m_Solver.End();
        }

        for (size_t i = 0; i < GetRigidBodyCount(); i++) //assign updated state to rigid bodies
        {
            m_RigidBodies[i]->Velocity = m_State.Velocity[i];
            m_RigidBodies[i]->Position = m_State.Position[i];

            m_RigidBodies[i]->AngularVelocity = m_State.AngularVelocity[i];
            m_RigidBodies[i]->Theta = m_State.Theta[i];
        }

        for (Constraint* constraint : m_Constraints) //assign updated state to constraints
        {
            for (size_t i = 0, c_i = 0; i < constraint->GetConstraintCount(); i++, c_i++)
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

    size_t RigidBodySystem::GetTotalConstraintCount() const
    {
        size_t total = 0;

        for (Constraint* c : m_Constraints)
            total += c->GetConstraintCount();

        return total;
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
        const size_t n = GetRigidBodyCount();

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
        for (size_t i = 0; i < GetRigidBodyCount(); i++)
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
        size_t n = GetRigidBodyCount();
        size_t m = GetConstraintCount();
        size_t m_t = GetTotalConstraintCount();

        //populate / initialize vectors and matrices
        m_MatricesData.qdot.Resize(3 * n, 1);

        for (size_t i = 0; i < n; i++)
        {
            m_MatricesData.qdot[i * 3 + 0][0] = m_State.Velocity[i].x;
            m_MatricesData.qdot[i * 3 + 1][0] = m_State.Velocity[i].y;
            m_MatricesData.qdot[i * 3 + 2][0] = m_State.AngularVelocity[i];
        }

        //m_MatricesData.SparseJacobian.reserve(m);
        //m_MatricesData.JacobianDot.reserve(m);
        m_MatricesData.Q.Initialize(n * 3, 1);
        m_MatricesData.C_ks.Initialize(m_t, 1);
        m_MatricesData.C_kd.Initialize(m_t, 1);
        m_MatricesData.C.Initialize(m_t, 1);

        //caluclate constraints and store them in respective matrices
        ConstraintOutput constraintSlice;
        size_t currentConstraintIndex = 0;
        size_t currentBodyIndex = 0;
        size_t currentIndex = 0;
        for (Constraint* c : m_Constraints)
        {
            c->Calculate(constraintSlice, &m_State);

            Matrices::BlockMatrix currentBlock;
            currentBlock.BlockJacobian = constraintSlice.J;
            currentBlock.i = currentConstraintIndex;
            currentBlock.j = currentBodyIndex;
            currentBlock.rows = c->GetConstraintCount();
            currentBlock.columns = c->GetBodyCount();
            currentConstraintIndex += c->GetConstraintCount();
            currentBodyIndex += c->GetBodyCount();

            m_MatricesData.SparseJacobian.push_back(currentBlock);

            currentBlock.BlockJacobian = constraintSlice.Jdot;
            m_MatricesData.JacobianDot.push_back(currentBlock);

            for (size_t i = 0; i < c->GetConstraintCount(); i++, currentIndex++)
            {
                m_MatricesData.C[currentIndex][0] = constraintSlice.C[i][0];
                m_MatricesData.C_ks[currentIndex][0] = constraintSlice.ks[i][0];
                m_MatricesData.C_kd[currentIndex][0] = constraintSlice.kd[i][0];
            }
        }

        Matrix Cdot;
        Cdot.Initialize(n * 3, 1);
        for (auto& block : m_MatricesData.SparseJacobian)
        {
            for (size_t i = 0; i < block.rows; i++)
            {
                for (size_t j = 0; j < m_MatricesData.qdot.Columns(); j++)
                {
                    for (size_t k = 0; k < block.columns; k++)
                        Cdot[block.i + i][j] += block.BlockJacobian[i][k] * m_MatricesData.qdot[block.i + i][j];
                }
            }

        }

        for (size_t i = 0; i < m_t; i++)
        {
            m_MatricesData.C_ks[i][0] *= Cdot[i][0];
            m_MatricesData.C_kd[i][0] *= m_MatricesData.C[i][0];
        }

        for (size_t i = 0; i < n; i++)
        {
            m_MatricesData.Q[i * 3 + 0][0] = m_State.Force[i].x;
            m_MatricesData.Q[i * 3 + 1][0] = m_State.Force[i].y;
            m_MatricesData.Q[i * 3 + 2][0] = m_State.Torque[i];
        }

        //Matrix WQ = m_MatricesData.Q.ScaleLeftDiagonal(m_MatricesData.W);
        m_MatricesData.Q.ScaleLeftDiagonal(m_MatricesData.W); //W * Q

        //set up matrix equation
        Matrix JWQ;
        JWQ.Initialize(n * 3, 1);
        for (auto& block : m_MatricesData.SparseJacobian)
        {
            for (size_t i = 0; i < block.rows; i++)
            {
                for (size_t j = 0; j < m_MatricesData.Q.Columns(); j++)
                {
                    for (size_t k = 0; k < block.columns; k++)
                        JWQ[block.i + i][j] += block.BlockJacobian[i][k] * m_MatricesData.Q[block.i + i][j];
                }
            }
            
        }

        Matrix JdotQdot;
        JdotQdot.Initialize(n * 3, 1);
        for (auto& block : m_MatricesData.JacobianDot)
        {
            for (size_t i = 0; i < block.rows; i++)
            {
                for (size_t j = 0; j < m_MatricesData.qdot.Columns(); j++)
                {
                    for (size_t k = 0; k < block.columns; k++)
                        JdotQdot[block.i + i][j] += block.BlockJacobian[i][k] * m_MatricesData.qdot[block.i + i][j];
                }
            }

        }

        Matrix right = - JdotQdot - JWQ;
        //solve matrix equation


        //disperse matrices to state

    }
}