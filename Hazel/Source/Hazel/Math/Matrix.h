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

/*
TODO:
Done: Solve the matrix equation (requires SLE solver)
Done: SLE solver (conjugate gradient method) can be optimized
Done: Figure out creation of objects 
Entities contain the constraints
Better ODE solver (RK4)
more constraint classes



     

*/

