#pragma once

#include <iostream> //temp for debug
#include <iomanip>
namespace Hazel::Math
{
    /*
    ROW MAJOR
       _n_n_n_
    m | 0 1 2 |
    m | 3 4 5 | iTh row i = 1
    m |_6_7_8_|
        jTh column j = 0
    */

	class Matrix
	{
    public:
        Matrix();
        Matrix(size_t rows, size_t columns, double value = 0.0);
        Matrix(const Matrix&);
        ~Matrix();

        constexpr size_t Rows() const { return m_Rows; }
        constexpr size_t Columns() const { return m_Columns; }

        void Resize(size_t rows, size_t columns);
        void Initialize(size_t rows, size_t columns, double value = 0.0);
        void InsertMatrix(size_t row, size_t column, const Matrix& subMatrix);

        //operators
        FORCEINLINE double* operator[](size_t row) const { HZ_CORE_ASSERT(row < m_Rows); return row * m_Columns + m_Matrix.get(); }
        FORCEINLINE double& operator()(size_t row, size_t column) const
        { 
            HZ_CORE_ASSERT(row < m_Rows || column < m_Columns);  
            return m_Matrix[row * m_Columns + column]; 
        }

        FORCEINLINE Matrix& operator=(const Matrix& other)
        {
            m_Rows = other.m_Rows;
            m_Columns = other.m_Columns;

            m_Matrix = CreateScope<double[]>(other.m_Rows * other.m_Columns);

            for (size_t i = 0; i < m_Rows; i++)
            {
                for (size_t j = 0; j < m_Columns; j++)
                {
                    (*this)[i][j] = other[i][j];
                }
            }

            return *this;
        }
        
        Matrix operator*(Matrix& B) { return Multiply(B); }
        inline Matrix operator*(double scale) { return Scale(*this, scale); }
        Matrix operator+(Matrix& B) { return Add(B); }
        Matrix operator-(Matrix& B) { return Add(-B); }
        Matrix operator-() { return Negate(); } 
        Matrix& operator+=(const Matrix& B) { return AddToThis(B); }


        //Debug methods
        void Print() 
        {
            for (size_t i = 0; i < m_Rows; i++)
            {
                for (size_t j = 0; j < m_Columns; j++)
                {
                    std::cout << std::setw(14) << std::left << (*this)[i][j] << " ";
                }
                std::cout << '\n';
            }
            std::cout << "\n";
        }
    
        //Matrix operations
        Matrix ScaleRightDiagonal(const Matrix& vector);
        Matrix ScaleLeftDiagonal(const Matrix& vector);
        Matrix Transpose();
        Matrix TransposeMultiply(const Matrix& matrix);
        Matrix Multiply(const Matrix &B);

        //Vector operations
        double Magnitude() const;
        double MagnitudeSquared() const;
        double Dot(const Matrix& vector) const;

        //Matrix and vector
        Matrix Add(const Matrix& B);
        Matrix& AddToThis(const Matrix& B);
        Matrix Scale(const Matrix& B, double scale);
        Matrix Negate();

    private:
        friend Matrix operator* (double scale, Matrix& mat);
        size_t m_Rows, m_Columns;
        Scope<double[]> m_Matrix;
    };

    inline Matrix operator*(double scale, Matrix& mat) { return mat.Scale(mat, scale); } //since this definition is inside a header file it needs to be marked as inline
}

#if 0 //Notes

struct IntermediateValues 
{
    //J is the Jocobian of C where C is a global state vector of all Constriant functions
    SparseMatrix<3> J_sparse, J_dot_sparse, sreg0;
    Matrix J_T; //jacobian transpose J^T
    Matrix M, M_inv; //mass and inverse mass matrix M and W (really just a vector)
    Matrix C;        //constraint vector
    Matrix ks, kd;   //vectors of damping constants
    Matrix q_dot;    //vector of initally legal velocities
    //Q vector of appilied forces 
    //Qhat vector of legal forces (ones that satisfy Cdotdot = 0) Qhat = J^T * lambda
    //
    //q vector of positions (assumed to be initially legal)
    //qdot vector of velocities (assumed to be initially legal)
    Matrix reg0, reg1, reg2;

    Matrix right; //right hand side of the matrix equation (− ̇J* ̇q − JWQ − ks*C − kd*Cdot)
    Matrix F_ext, F_C, R;

    // Results
    Matrix lambda;
} m_iv;

void atg_scs::GenericRigidBodySystem::processConstraints()
{
    //JWJT ∏ = − ̇J  ̇q − JWQ − k s C − k d  ̇C
    m_iv.F_ext.initialize(1, 3 * n, 0.0);
    for (int i = 0; i < n; ++i) 
    {
        m_iv.F_ext.set(0, i * 3 + 0, m_state.f_x[i]);
        m_iv.F_ext.set(0, i * 3 + 1, m_state.f_y[i]);
        m_iv.F_ext.set(0, i * 3 + 2, m_state.t[i]);
    }

    m_iv.F_ext.leftScale(m_iv.M_inv, &m_iv.reg2); //W * Q
    m_iv.J_sparse.multiply(m_iv.reg2, &m_iv.reg0); //J * (W * Q)

    m_iv.J_dot_sparse.multiply(m_iv.q_dot, &m_iv.reg2); //Jdot * qdot
    m_iv.reg2.negate(&m_iv.reg1); //-Jdot * qdot

    m_iv.reg1.subtract(m_iv.reg0, &m_iv.reg2); //-JDot * qdot - JWQ
    m_iv.reg2.subtract(m_iv.ks, &m_iv.reg0);   //-JDot * qdot - JWQ - Cks
    m_iv.reg0.subtract(m_iv.kd, &m_iv.right);  //-JDot * qdot - JWQ - Cks - Cdot*kd

    //solve matrix equation A lambda = B
    const bool solvable =//(Jacobian,     W         ,− ̇J  ̇q − JWQ, lambda, previous lambda)
    assert(solvable);
        m_sleSolver->solve(m_iv.J_sparse, m_iv.M_inv, m_iv.right, &m_iv.lambda, &m_iv.lambda);

    // Constraint force derivation
    //  R = J_T * lambda_scale //R is All vectors that satisfy ˆQ ·  ̇x = 0, ∀ ̇x | J ̇x = 0 for all xdot such that J xdot = 0
    //  => transpose(J) * transpose(transpose(lambda_scale)) = R
    //  => transpose(lambda_scale * J) = R
    //  => transpose(J.leftScale(lambda_scale)) = R

    m_iv.J_sparse.leftScale(m_iv.lambda, &m_iv.sreg0); //

    for (int i = 0; i < m_f; ++i) 
    {
        for (int j = 0; j < 2; ++j) //2 the number of bodies per constraint
        {
            m_state.r_x[i * 2 + j] = m_iv.sreg0.get(i, j, 0);
            m_state.r_y[i * 2 + j] = m_iv.sreg0.get(i, j, 1);
            m_state.r_t[i * 2 + j] = m_iv.sreg0.get(i, j, 2);
        }
    }

    for (int i = 0; i < n; ++i) 
    {
        m_state.a_x[i] = m_iv.F_ext.get(0, i * 3 + 0);
        m_state.a_y[i] = m_iv.F_ext.get(0, i * 3 + 1);
        m_state.a_theta[i] = m_iv.F_ext.get(0, i * 3 + 2);
    }

    for (int i = 0, j_f = 0; i < m; ++i) 
    {
        Constraint* constraint = m_constraints[i];

        const int n_f = constraint->getConstraintCount();
        for (int j = 0; j < n_f; ++j, ++j_f) 
        {
            for (int k = 0; k < constraint->m_bodyCount; ++k) 
            {
                const int body = constraint->m_bodies[k]->index;
                m_state.a_x[body] += m_state.r_x[j_f * 2 + k];
                m_state.a_y[body] += m_state.r_y[j_f * 2 + k];
                m_state.a_theta[body] += m_state.r_t[j_f * 2 + k];
            }
        }
    }

    for (int i = 0; i < n; ++i) 
    {
        const double invMass = m_iv.M_inv.get(0, i * 3 + 0);
        const double invInertia = m_iv.M_inv.get(0, i * 3 + 2);

        m_state.a_x[i] *= invMass;
        m_state.a_y[i] *= invMass;
        m_state.a_theta[i] *= invInertia;
    }
}

Vector Qhat = SparseJacobianTranspose * m_MatricesData.Lambda;

for (size_t i = 0; i < n; i++)
{
    m_State.ConstraintForce[i].x = Qhat[i * 3 + 0][0];
    m_State.ConstraintForce[i].y = Qhat[i * 3 + 1][0];
    m_State.ConstraintTorque[i] = Qhat[i * 3 + 2][0];
}
//POS 
Vector Qhat = m_MatricesData.SparseJacobian.ScaleLeftDiagonal(m_MatricesData.Lambda);

for (size_t i = 0; i < m_t; i++)
{
    for (size_t j = 0; j < 2; j++)
    {
        m_State.ConstraintForce[i * 2 + j].x = Qhat[i][j * 3 + 0];
        m_State.ConstraintForce[i * 2 + j].y = Qhat[i][j * 3 + 1];
        m_State.ConstraintTorque[i * 2 + j] = Qhat[i][j * 3 + 2];
    }
}

for (Constraint* c : m_Constraints)
{
    size_t constraintIndex = 0;
    for (size_t i = 0; i < c->GetConstraintCount(); i++, constraintIndex++)
    {
        for (size_t j = 0; j < c->GetBodyCount(); j++)
        {
            size_t index = c->GetBody(j)->Index;
            m_State.Acceleration[index].x += m_State.ConstraintForce[constraintIndex * 2 + j].x;
            m_State.Acceleration[index].y += m_State.ConstraintForce[constraintIndex * 2 + j].y;
            m_State.Torque[index] += m_State.ConstraintTorque[constraintIndex * 2 + j];
        }
    }
}

glm::dvec2 locallink = testbody1->WorldToLocal(lastPosition);
Enyoo::LinkConstraint* link2 = new Enyoo::LinkConstraint;
m_ActiveScene->m_NewBodySystem->AddConstraint(link2);
link2->SetFirstBody(testbody1);
link2->SetSecondBody(testbody2);
link2->SetFirstBodyLocal(locallink);
link2->SetSecondBodyLocal({ 0.0, 0.0 })

#endif

/*
TODO:
Solve the matrix equation (requires SLE solver)
Done: SLE solver (conjugate gradient method)
Figure out creation of objects (entities with constraints)
Better ODE solver (RK4)
more constraint classes

const double dx2 = 8.0 / 3.0 - lastPosition.x;
        const double dy2 = 1.0 - lastPosition.y;
        const double length2 = glm::sqrt(dx2 * dx2 + dy2 * dy2);

        const double theta2 = (dy2 > 0) ? glm::acos(dx2 / length2) : glm::two_pi<double>() - glm::acos(dx2 / length2);
        m_ActiveScene->m_NewBodySystem->AddRigidBody(testbody3);
        testbody3->Theta = theta2;

        glm::dvec2 world2 = testbody3->LocalToWorld({ -length2 / 2, 0 });
        testbody3->Position = lastPosition - world2;
        testbody3->Mass = length * 1.0;

        glm::dvec2 locallink2 = testbody1->WorldToLocal(lastPosition);
        Enyoo::LinkConstraint* link1 = new Enyoo::LinkConstraint;
        m_ActiveScene->m_NewBodySystem->AddConstraint(link1);
        link1->SetFirstBody(testbody3);
        link1->SetSecondBody(testbody1);
        link1->SetFirstBodyLocal({ -length / 2, 0 });
        link1->SetSecondBodyLocal(locallink2);

        lastPosition = testbody3->LocalToWorld({ length / 2, 0 });
*/

