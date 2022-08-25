#include <hzpch.h>

#include "RigidBodySystem.h"

namespace Enyoo
{
    void RigidBodySystem::Init()
    {

    }

    void RigidBodySystem::Step(double dt, uint32_t steps)
    {
        PopulateSystemState();
        PopulateMassMatrices(m_MatricesData.Mass, m_MatricesData.W);

        for (int i = 0; i < steps; i++)
        {
            m_TimeIntegrator.Start(m_State, dt / steps);

            while (true)
            {
                const bool done = m_TimeIntegrator.Step(m_State);
                UpdateForces();
                ResolveConstraints();
                m_TimeIntegrator.Integrate(m_State);

                if (done) break;
            }

            m_TimeIntegrator.End();
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

    void RigidBodySystem::RemoveForceGen(ForceGenerator* forceGen)
    {
    }

    void RigidBodySystem::RemoveConstraint(Constraint* constraint)
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
        m_State.Resize(GetRigidBodyCount(), GetTotalConstraintCount());

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
     
            massInverse[i * 3 + 0][0] = 1 / m_RigidBodies[i]->Mass;
            massInverse[i * 3 + 1][0] = 1 / m_RigidBodies[i]->Mass;
            massInverse[i * 3 + 2][0] = 1 / m_RigidBodies[i]->MomentInertia;
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
        static int iteration = 0;

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
        m_MatricesData.SparseJacobian.Initialize(m_t, n * 3);
        m_MatricesData.SparseJacobianDot.Initialize(m_t, n * 3);
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
            //currentBlock.BlockJacobian = constraintSlice.J;
            currentBlock.i = currentConstraintIndex;
            currentBlock.j = currentBodyIndex;
            currentBlock.rows = c->GetConstraintCount();
            currentBlock.columns = c->GetBodyCount();

            //m_MatricesData.SparseJacobian.push_back(currentBlock);

            m_MatricesData.SparseJacobian.InsertMatrix(currentConstraintIndex, currentBodyIndex, constraintSlice.J);
            //constraintSlice.J.Print();

            m_MatricesData.SparseJacobianDot.InsertMatrix(currentConstraintIndex, currentBodyIndex, constraintSlice.Jdot);

            //currentBlock.BlockJacobian = constraintSlice.Jdot;
            //m_MatricesData.JacobianDot.push_back(currentBlock);

            for (size_t i = 0; i < c->GetConstraintCount(); i++, currentIndex++)
            {
                m_MatricesData.C[currentIndex][0] = constraintSlice.C[i][0];
                m_MatricesData.C_ks[currentIndex][0] = constraintSlice.ks[i][0];
                m_MatricesData.C_kd[currentIndex][0] = constraintSlice.kd[i][0];
            }

            currentConstraintIndex += c->GetConstraintCount();
            currentBodyIndex += 3 * c->GetBodyCount();
        }
        //m_MatricesData.SparseJacobian.Print();

        //Matrix Cdot;
        //Cdot.Initialize(n * 3, 1);
        //for (const auto& block : m_MatricesData.SparseJacobian)
        //{
        //    for (size_t i = 0; i < block.rows; i++)
        //    {
        //        for (size_t j = 0; j < m_MatricesData.qdot.Columns(); j++)
        //        {
        //            for (size_t k = 0; k < block.columns; k++)
        //                Cdot[block.i + i][j] += block.BlockJacobian[i][k] * m_MatricesData.qdot[block.i + i][j];
        //        }
        //    }
        //
        //}

        for (size_t i = 0; i < m_t; i++)
        {
            //m_MatricesData.C_ks[i][0] *= Cdot[i][0];
            m_MatricesData.C_kd[i][0] *= m_MatricesData.C[i][0];
        }

        for (size_t i = 0; i < n; i++)
        {
            m_MatricesData.Q[i * 3 + 0][0] = m_State.Force[i].x;
            m_MatricesData.Q[i * 3 + 1][0] = m_State.Force[i].y;
            m_MatricesData.Q[i * 3 + 2][0] = m_State.Torque[i];
        }

        m_MatricesData.Q.ScaleLeftDiagonal(m_MatricesData.W); //W * Q

        //set up matrix equation
        Matrix JWQ = m_MatricesData.SparseJacobian * m_MatricesData.Q;
        Matrix JdotQdot = m_MatricesData.SparseJacobianDot * m_MatricesData.qdot;
        Matrix SparseJacobianTranspose = m_MatricesData.SparseJacobian.Transpose();
        Vector b = - JdotQdot - JWQ;
        Hazel::Timer fulltime;
        Matrix A = m_MatricesData.SparseJacobian * SparseJacobianTranspose.ScaleLeftDiagonal(m_MatricesData.W);
        //A.Print();
        //solve matrix equation
        //HZ_CORE_TRACE("Iteration: {0} Time: {1}ms", iteration++, fulltime.ElapsedMilliseconds());
        const bool solved = m_LinearEquationSolver.Solve(A, b, &m_MatricesData.Lambda);
        HZ_CORE_ASSERT(solved);

        //disperse matrices to state
        //adotdot = (AppiliedForce + ConstraintForce) / m
        Vector Qhat = SparseJacobianTranspose * m_MatricesData.Lambda;
        for (size_t i = 0; i < n; i++)
        {
            m_State.ConstraintForce[i].x = Qhat[i * 3 + 0][0];
            m_State.ConstraintForce[i].y = Qhat[i * 3 + 1][0];
            m_State.ConstraintTorque[i]  = Qhat[i * 3 + 2][0];
        }
        //m_MatricesData.Q.Print();
        //Qhat.Print();
#if 1
        for (size_t i = 0; i < n; i++)
        {
            m_State.Acceleration[i].x      = m_MatricesData.Q[i * 3 + 0][0] + Qhat[i * 3 + 0][0];
            m_State.Acceleration[i].y      = m_MatricesData.Q[i * 3 + 1][0] + Qhat[i * 3 + 1][0];
            m_State.AngularAcceleration[i] = m_MatricesData.Q[i * 3 + 2][0] + Qhat[i * 3 + 2][0];
        }
#endif
#if 0
        for (Constraint* c : m_Constraints)
        {
            size_t constraintIndex = 0;
            for (size_t i = 0; i < c->GetConstraintCount(); i++, constraintIndex++)
            {
                for (size_t j = 0; j < c->GetBodyCount(); j++)
                {
                    size_t index = c->GetBody(j)->Index;
                    m_State.Acceleration[index].x += m_State.ConstraintForce[constraintIndex + j].x;
                    m_State.Acceleration[index].y += m_State.ConstraintForce[constraintIndex + j].y;
                    m_State.Torque[index]         += m_State.ConstraintTorque[constraintIndex + j];
                }
            }
        }
#endif
#if 0
        size_t i = 0;
        size_t j = 1;
        size_t k = 2;
        size_t l = 0;
        size_t mt = 1;
        size_t nt = 2;
        size_t o = 0;
        size_t p = 1;
        size_t q = 2;

        m_State.Acceleration[i].x = m_MatricesData.Q[l * 3 + 0][0] + Qhat[o * 3 + 0][0];
        m_State.Acceleration[i].y = m_MatricesData.Q[l * 3 + 1][0] + Qhat[o * 3 + 1][0];
        m_State.AngularAcceleration[i] = m_MatricesData.Q[l * 3 + 2][0] + Qhat[o * 3 + 2][0];
        m_State.Acceleration[j].x = m_MatricesData.Q[mt * 3 + 0][0] + Qhat[p * 3 + 0][0];
        m_State.Acceleration[j].y = m_MatricesData.Q[mt * 3 + 1][0] + Qhat[p * 3 + 1][0];
        m_State.AngularAcceleration[j] = m_MatricesData.Q[mt * 3 + 2][0] + Qhat[p * 3 + 2][0];
        m_State.Acceleration[k].x = m_MatricesData.Q[nt * 3 + 0][0] + Qhat[q * 3 + 0][0];
        m_State.Acceleration[k].y = m_MatricesData.Q[nt * 3 + 1][0] + Qhat[q * 3 + 1][0];
        m_State.AngularAcceleration[k] = m_MatricesData.Q[nt * 3 + 2][0] + Qhat[q * 3 + 2][0];
#endif


        for (size_t i = 0; i < n; i++)
        {
            m_State.Acceleration[i] *= m_MatricesData.W[i * 3 + 0][0];
            m_State.AngularAcceleration[i] *= m_MatricesData.W[i * 3 + 2][0];
        }
    }
}