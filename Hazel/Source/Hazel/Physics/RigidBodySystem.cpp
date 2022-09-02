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

        for (uint32_t i = 0; i < steps; i++)
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

        for (size_t i = 0; i < GetRigidBodyCount(); i++) // assign updated state to rigid bodies
        {
            m_RigidBodies[i]->Velocity = m_State.Velocity[i];
            m_RigidBodies[i]->Position = m_State.Position[i];

            m_RigidBodies[i]->AngularVelocity = m_State.AngularVelocity[i];
            m_RigidBodies[i]->Theta = m_State.Theta[i];
        }

        //for (Constraint* constraint : m_Constraints) // assign updated state to constraints
        //{
        //    for (size_t i = 0, c_i = 0; i < constraint->GetConstraintCount(); i++, c_i++)
        //    {
        //        for (size_t j = 0; j < constraint->GetBodyCount(); j++)
        //        {
        //            constraint->ConstraintForceX[i][j] = m_State.ConstraintForce[c_i].x;
        //            constraint->ConstraintForceY[i][j] = m_State.ConstraintForce[c_i].y;
        //            constraint->ConstraintTorque[i][j] = m_State.ConstraintTorque[c_i];
        //        }
        //    }
        //}
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

        for (size_t i = 0, constraintCount = 0; i < GetConstraintCount(); i++)
        {
            constraintCount += m_Constraints[i]->GetConstraintCount();
        }
    }

    void RigidBodySystem::PopulateMassMatrices(Matrix& mass, Matrix& massInverse)
    {
        const size_t n = GetRigidBodyCount();

        mass.Resize(n * 3, 1);
        massInverse.Resize(n * 3, 1);

        for (size_t i = 0; i < n; i++)
        {
            mass[i * 3 + 0][0] = m_RigidBodies[i]->Mass;
            mass[i * 3 + 1][0] = m_RigidBodies[i]->Mass;
            mass[i * 3 + 2][0] = m_RigidBodies[i]->MomentInertia;
     
            massInverse[i * 3 + 0][0] = 1.0 / m_RigidBodies[i]->Mass;
            massInverse[i * 3 + 1][0] = 1.0 / m_RigidBodies[i]->Mass;
            massInverse[i * 3 + 2][0] = 1.0 / m_RigidBodies[i]->MomentInertia;
        }
    }

    void RigidBodySystem::UpdateForces()
    {
        // zero out forces
        for (size_t i = 0; i < GetRigidBodyCount(); i++)
        {
            m_State.Force[i] = glm::dvec2{ 0.0 };
            m_State.Torque[i] = 0.0;
        }
        // loop through force generators and apply their force to the state
        for (ForceGenerator* forceGen : m_ForceGenerators)
            forceGen->ApplyForce(m_State);
    }

    void RigidBodySystem::ResolveConstraints()
    {
        size_t n = GetRigidBodyCount();
        size_t m = GetConstraintCount();
        size_t m_t = GetTotalConstraintCount();

        // populate vectors and matrices
        m_MatricesData.qdot.Resize(3 * n, 1);

        for (size_t i = 0; i < n; i++)
        {
            m_MatricesData.qdot[i * 3 + 0][0] = m_State.Velocity[i].x;
            m_MatricesData.qdot[i * 3 + 1][0] = m_State.Velocity[i].y;
            m_MatricesData.qdot[i * 3 + 2][0] = m_State.AngularVelocity[i];
        }

        m_MatricesData.SparseJacobian.Initialize(m_t, n * 3);
        m_MatricesData.SparseJacobianDot.Initialize(m_t, n * 3);
        m_MatricesData.Q.Initialize(n * 3, 1);
        m_MatricesData.ks.Initialize(m_t, 1);
        m_MatricesData.kd.Initialize(m_t, 1);
        m_MatricesData.C.Initialize(m_t, 1);

        // caluclate constraints and store them in respective matrices
        std::unordered_map<size_t, size_t> indexMap;
        ConstraintOutput constraintSlice;
        size_t currentConstraintIndex = 0;
        size_t currentBodyIndex = 0;
        size_t currentIndex = 0;
        for (Constraint* c : m_Constraints)
        {
            c->Calculate(constraintSlice, &m_State);

            for (uint32_t i = 0; i < c->GetBodyCount(); i++)
            {
                const size_t index = c->GetBody(i)->Index;

                if (indexMap.count(index)) 
                {
                    m_MatricesData.SparseJacobian.InsertMatrix(currentConstraintIndex, indexMap.at(index), constraintSlice.J[i]);
                    m_MatricesData.SparseJacobianDot.InsertMatrix(currentConstraintIndex, indexMap.at(index), constraintSlice.Jdot[i]);
                    currentBodyIndex -= 3;
                }
                else
                {
                    indexMap[index] = currentBodyIndex;
                    m_MatricesData.SparseJacobian.InsertMatrix(currentConstraintIndex, currentBodyIndex, constraintSlice.J[i]);
                    m_MatricesData.SparseJacobianDot.InsertMatrix(currentConstraintIndex, currentBodyIndex, constraintSlice.Jdot[i]);
                }
                //constraintSlice.J[i].Print();
                //m_MatricesData.SparseJacobian.Print();
            }

            for (uint32_t i = 0; i < c->GetConstraintCount(); i++, currentIndex++)
            {
                m_MatricesData.C[currentIndex][0] = constraintSlice.C[i][0];
                m_MatricesData.ks[currentIndex][0] = constraintSlice.ks[i][0];
                m_MatricesData.kd[currentIndex][0] = constraintSlice.kd[i][0];
            }

            currentConstraintIndex += c->GetConstraintCount();
            currentBodyIndex += 3 * c->GetBodyCount();
        }

        Matrix Cdot = m_MatricesData.SparseJacobian * m_MatricesData.qdot;
        for (size_t i = 0; i < m_t; i++)
        {
            m_MatricesData.ks[i][0] *= m_MatricesData.C[i][0];
            m_MatricesData.kd[i][0] *= Cdot[i][0];
        }

        for (size_t i = 0; i < n; i++)
        {
            m_MatricesData.Q[i * 3 + 0][0] = m_State.Force[i].x;
            m_MatricesData.Q[i * 3 + 1][0] = m_State.Force[i].y;
            m_MatricesData.Q[i * 3 + 2][0] = m_State.Torque[i];
        }
       
        // set up matrix equation

        Matrix WQ = m_MatricesData.Q.ScaleLeftDiagonal(m_MatricesData.W); // W * Q
        Matrix JWQ = m_MatricesData.SparseJacobian * WQ;
        Matrix JdotQdot = m_MatricesData.SparseJacobianDot * m_MatricesData.qdot;
        JdotQdot = -1 * JdotQdot;
        JdotQdot -= JWQ;
        JdotQdot -= m_MatricesData.ks;
        JdotQdot -= m_MatricesData.kd;
        Matrix SparseJacobianTranspose = m_MatricesData.SparseJacobian.Transpose();
        Matrix WJT = SparseJacobianTranspose.ScaleLeftDiagonal(m_MatricesData.W);
        Matrix A = m_MatricesData.SparseJacobian * WJT;

        // solve matrix equation
        const bool solved = m_LinearEquationSolver.Solve(A, JdotQdot, &m_MatricesData.lambda);
        HZ_CORE_ASSERT(solved); 

        // disperse matrices to state
        Vector Qhat = SparseJacobianTranspose * m_MatricesData.lambda;

        //for (size_t i = 0; i < n; i++)
        //{
        //    m_State.ConstraintForce[i].x = Qhat[i * 3 + 0][0];
        //    m_State.ConstraintForce[i].y = Qhat[i * 3 + 1][0];
        //    m_State.ConstraintTorque[i] = Qhat[i * 3 + 2][0];
        //}

        // xdotdot = (AppiliedForce + ConstraintForce) / m
        for (size_t i = 0; i < n; i++)
        {
            m_State.Acceleration[i].x      = m_MatricesData.Q[i * 3 + 0][0] + Qhat[i * 3 + 0][0];
            m_State.Acceleration[i].y      = m_MatricesData.Q[i * 3 + 1][0] + Qhat[i * 3 + 1][0];
            m_State.AngularAcceleration[i] = m_MatricesData.Q[i * 3 + 2][0] + Qhat[i * 3 + 2][0];
        }
        
        for (size_t i = 0; i < n; i++)
        {
            m_State.Acceleration[i] *= m_MatricesData.W[i * 3 + 0][0];
            m_State.AngularAcceleration[i] *= m_MatricesData.W[i * 3 + 2][0];
        }
    }
}