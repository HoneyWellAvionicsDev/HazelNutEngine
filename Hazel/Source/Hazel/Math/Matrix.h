#pragma once

#include <iostream> //temp for debug
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
        Matrix& operator+=(Matrix B) { return Add(B); }


        //Debug methods
        void Print() 
        {
            for (size_t i = 0; i < m_Rows; i++)
            {
                for (size_t j = 0; j < m_Columns; j++)
                {
                    std::cout << (*this)[i][j] << '\t';
                }
                std::cout << '\n';
            }
            std::cout << "\n";
        }
    
        //Matrix operations
        Matrix& ScaleRightDiagonal(const Matrix& vector);
        Matrix& ScaleLeftDiagonal(const Matrix& vector);
        Matrix Transpose();
        Matrix TransposeMultiply(const Matrix& matrix);
        Matrix Multiply(const Matrix &B);

        //Vector operations
        double Magnitude() const;
        double MagnitudeSquared() const;
        double Dot(const Matrix& vector) const;

        //Matrix and vector
        Matrix& Add(const Matrix& B);
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
#pragma region matricies
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

struct Constraint::Output 
{
    double C[MaxConstraintCount];
    double J[MaxConstraintCount][3 * MaxBodyCount];
    double J_dot[MaxConstraintCount][3 * MaxBodyCount];
    double v_bias[MaxConstraintCount];
    double limits[MaxConstraintCount][2];
    double ks[MaxConstraintCount]; //these should be std::vectors
    double kd[MaxConstraintCount];
};
#pragma endregion

void atg_scs::GenericRigidBodySystem::processConstraints()
{
    const int n = getRigidBodyCount(); //for every particle, there are three things to keep track of: x, y, theta
    const int m_f = getFullConstraintCount(); //sum of constraints each indivual constraint contains
    const int m = getConstraintCount(); //#number of contraints placed on the system?

    m_iv.q_dot.resize(1, n * 3); //three slots so that we can store the x, y, and theta velocities
    for (int i = 0; i < n; ++i) 
    {
        m_iv.q_dot.set(0, i * 3 + 0, m_state.v_x[i]);
        m_iv.q_dot.set(0, i * 3 + 1, m_state.v_y[i]);
        m_iv.q_dot.set(0, i * 3 + 2, m_state.v_theta[i]);
    }

    m_iv.J_sparse.initialize(3 * n, m_f);
    m_iv.J_dot_sparse.initialize(3 * n, m_f);
    m_iv.ks.initialize(1, m_f);
    m_iv.kd.initialize(1, m_f);
    m_iv.C.initialize(1, m_f);

    Constraint::Output constraintOutput;
    for (int j = 0, j_f = 0; j < m; ++j) //for every constraint object in the system
    {
        m_constraints[j]->calculate(&constraintOutput, &m_state); // for all contraints calculate J and J dot based on their constraint functions
                                                                  //where J is the jacobian of the constraint function (with respect to system state)

        const int n_f = m_constraints[j]->getConstraintCount();//a single constraint will add a block for every body it depends on
        for (int k = 0; k < n_f; ++k, ++j_f) //for all blocks in the master matrix (for all constraints in a single constraint)
        {
            for (int i = 0; i < m_constraints[j]->m_bodyCount; ++i) //for each body a given constraint object concerns (max 2)
            {
                const int index = m_constraints[j]->m_bodies[i]->index;

                if (index == -1) continue;

                m_iv.J_sparse.setBlock(j_f, i, index);//store bodies index in master matrix
                m_iv.J_dot_sparse.setBlock(j_f, i, index);
            }

            for (int i = 0; i < m_constraints[j]->m_bodyCount * 3; ++i) 
            {
                const int index = m_constraints[j]->m_bodies[i / 3]->index;

                if (index == -1) continue;

                m_iv.J_sparse.set(j_f, i / 3, i % 3, //take the precomputed J from each constraint and add a block to master matrix
                    constraintOutput.J[k][i]);       //we need to make a block for every body that the constraint depends on

                m_iv.J_dot_sparse.set(j_f, i / 3, i % 3,
                    constraintOutput.J_dot[k][i]);

                m_iv.ks.set(0, j_f, constraintOutput.ks[k]);
                m_iv.kd.set(0, j_f, constraintOutput.kd[k]);
                m_iv.C.set(0, j_f, constraintOutput.C[k]);
            }
        }
    }

    m_iv.J_sparse.multiply(m_iv.q_dot, &m_iv.reg0);
    for (int i = 0; i < m_f; ++i) 
    {
        m_iv.kd.set(0, i, m_iv.kd.get(0, i) * m_iv.reg0.get(0, i));
        m_iv.ks.set(0, i, m_iv.ks.get(0, i) * m_iv.C.get(0, i));
    }

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
    m_iv.reg0.subtract(m_iv.kd, &m_iv.right);  ////-JDot * qdot - JWQ - Cks - Cdot*kd

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

#endif

/*
TODO:
Solve the matrix equation (requires SLE solver)
Done: SLE solver (conjugate gradient method)
Figure out creation of objects (entities with constraints)
Better ODE solver (RK4)
more constraint classes
*/