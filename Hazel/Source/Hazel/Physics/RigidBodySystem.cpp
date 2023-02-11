#include <hzpch.h>

#include "RigidBodySystem.h"

namespace Enyoo
{
    void RigidBodySystem::Initialize()
    { //this needs to be called AFTER all constraints have been added to the phys system
        m_LinearEquationSolver.Initialize(GetTotalConstraintCount());
    }

    void RigidBodySystem::Step(double dt, uint32_t steps)
    {
        PopulateSystemState();
        PopulateMassMatrices(m_Matrices.Mass, m_Matrices.W);

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
    }

    void RigidBodySystem::AddRigidBody(RigidBody* body)
    {
        m_RigidBodies.push_back(body);
        HZ_CORE_TRACE("Body added to system");
        body->Index = m_RigidBodies.size() - 1;
    }

    void RigidBodySystem::AddForceGen(ForceGenerator* forceGen)
    {
        m_ForceGenerators.push_back(forceGen);
        forceGen->SetIndex(m_ForceGenerators.size() - 1);
    }

    void RigidBodySystem::AddConstraint(const Hazel::Ref<Constraint>& constraint)
    {
        m_Constraints.push_back(constraint);
        constraint->SetIndex(m_Constraints.size() - 1);
    }

    void RigidBodySystem::RemoveRigidBody(RigidBody* body)
    {
    }

    void RigidBodySystem::RemoveForceGen(ForceGenerator* forceGen)
    {
    }

    void RigidBodySystem::RemoveConstraint(const Hazel::Ref<Constraint>& constraint)
    {
    }

    size_t RigidBodySystem::GetTotalConstraintCount() const
    {
        size_t total = 0;

        for (Hazel::Ref<Constraint> c : m_Constraints)
            total += c->GetConstraintCount();

        return total;
    }

    double RigidBodySystem::GetTotalSystemEnergy() const
    {
        double total = 0.0;

        for (RigidBody* body : m_RigidBodies)
            total += body->CalculateEnergy();

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
        m_Matrices.qdot.Resize(3 * n, 1);

        for (size_t i = 0; i < n; i++)
        {
            m_Matrices.qdot[i * 3 + 0][0] = m_State.Velocity[i].x;
            m_Matrices.qdot[i * 3 + 1][0] = m_State.Velocity[i].y;
            m_Matrices.qdot[i * 3 + 2][0] = m_State.AngularVelocity[i];
        }

        m_Matrices.SparseJacobian.Initialize(m_t, n * 3);
        m_Matrices.SparseJacobianDot.Initialize(m_t, n * 3);
        m_Matrices.Q.Initialize(n * 3, 1);
        m_Matrices.ks.Initialize(m_t, 1);
        m_Matrices.kd.Initialize(m_t, 1);
        m_Matrices.C.Initialize(m_t, 1);

        // caluclate constraints and store them in respective matrices
        std::unordered_map<size_t, size_t> indexMap;
        ConstraintOutput constraintSlice;
        size_t currentConstraintIndex = 0;
        size_t currentBodyIndex = 0;
        size_t currentIndex = 0;

        for (Hazel::Ref<Constraint> constraint : m_Constraints)
        {
            constraint->Calculate(constraintSlice, &m_State);

            for (uint32_t i = 0; i < constraint->GetBodyCount(); i++)
            {
                const size_t index = constraint->GetBody(i)->Index;

                if (indexMap.count(index))
                {
                    m_Matrices.SparseJacobian.InsertMatrix(currentConstraintIndex, indexMap.at(index), constraintSlice.J[i]);
                    m_Matrices.SparseJacobianDot.InsertMatrix(currentConstraintIndex, indexMap.at(index), constraintSlice.Jdot[i]);
                }
                else
                {
                    m_Matrices.SparseJacobian.InsertMatrix(currentConstraintIndex, currentBodyIndex, constraintSlice.J[i]);
                    m_Matrices.SparseJacobianDot.InsertMatrix(currentConstraintIndex, currentBodyIndex, constraintSlice.Jdot[i]);

                    indexMap[index] = currentBodyIndex;
                    currentBodyIndex += 3;
                }
            }

            for (uint32_t i = 0; i < constraint->GetConstraintCount(); i++, currentIndex++)
            {
                m_Matrices.C[currentIndex][0] = constraintSlice.C[i][0];
                m_Matrices.ks[currentIndex][0] = constraintSlice.ks[i][0];
                m_Matrices.kd[currentIndex][0] = constraintSlice.kd[i][0];
            }

            currentConstraintIndex += constraint->GetConstraintCount();
        }
        //m_Matrices.SparseJacobian.Print();

        Matrix Cdot = m_Matrices.SparseJacobian * m_Matrices.qdot;
        for (size_t i = 0; i < m_t; i++)
        {
            m_Matrices.ks[i][0] *= m_Matrices.C[i][0];
            m_Matrices.kd[i][0] *= Cdot[i][0];
        }

        for (size_t i = 0; i < n; i++)
        {
            m_Matrices.Q[i * 3 + 0][0] = m_State.Force[i].x;
            m_Matrices.Q[i * 3 + 1][0] = m_State.Force[i].y;
            m_Matrices.Q[i * 3 + 2][0] = m_State.Torque[i];
        }
       
        // set up matrix equation
 
        Matrix::ScaleLeftDiagonal(m_Matrices.Q, m_Matrices.W, m_Matrices.WQ);
        Matrix::Multiply(m_Matrices.SparseJacobian, m_Matrices.WQ, m_Matrices.JWQ);
        Matrix::Multiply(m_Matrices.SparseJacobianDot, m_Matrices.qdot, m_Matrices.JdotQdot);

        m_Matrices.JdotQdot = -1 * m_Matrices.JdotQdot;
        m_Matrices.JdotQdot -= m_Matrices.JWQ;
        m_Matrices.JdotQdot -= m_Matrices.ks;
        m_Matrices.JdotQdot -= m_Matrices.kd;
        Matrix SparseJacobianTranspose = m_Matrices.SparseJacobian.Transpose();
        Matrix WJT = SparseJacobianTranspose.ScaleLeftDiagonal(m_Matrices.W);
        Matrix A = m_Matrices.SparseJacobian * WJT;
        
        // solve matrix equation
        m_LinearEquationSolver.Solve(A, m_Matrices.JdotQdot, m_Matrices.lambda);

        // disperse matrices to state
        m_Matrices.Qhat = SparseJacobianTranspose * m_Matrices.lambda;

        // xdotdot = (AppiliedForce + ConstraintForce) / m
        for (size_t i = 0; i < n; i++)
        {
            m_State.Acceleration[i].x      = m_Matrices.Q[i * 3 + 0][0] + m_Matrices.Qhat[i * 3 + 0][0];
            m_State.Acceleration[i].y      = m_Matrices.Q[i * 3 + 1][0] + m_Matrices.Qhat[i * 3 + 1][0];
            m_State.AngularAcceleration[i] = m_Matrices.Q[i * 3 + 2][0] + m_Matrices.Qhat[i * 3 + 2][0];
        }
        
        for (size_t i = 0; i < n; i++)
        {
            m_State.Acceleration[i] *= m_Matrices.W[i * 3 + 0][0];
            m_State.AngularAcceleration[i] *= m_Matrices.W[i * 3 + 2][0];
        }
    }
}