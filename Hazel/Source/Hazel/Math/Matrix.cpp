#include "hzpch.h"
#include "Matrix.h"

namespace Hazel::Math
{
	Matrix::Matrix()
		: m_Matrix(nullptr), m_Rows(0), m_Columns(0)
	{
	}

	Matrix::Matrix(size_t rows, size_t columns, double value)
		: m_Matrix(nullptr), m_Rows(rows), m_Columns(columns)
	{
		Initialize(rows, rows, value);
	}

	Matrix::Matrix(const Matrix& other)
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
	}

	Matrix::~Matrix()
	{
		//HZ_CORE_ASSERT(m_Matrix == nullptr);
	}

	void Matrix::Resize(size_t rows, size_t columns)
	{
		if (rows == m_Rows && columns == m_Columns)
			return;

		m_Matrix = CreateScope<double[]>(rows * columns);
		m_Rows = rows;
		m_Columns = columns;
	}

	void Matrix::Initialize(size_t rows, size_t columns, double value)
	{
		m_Matrix = CreateScope<double[]>(rows * columns);
		m_Rows = rows;
		m_Columns = columns;

		for (size_t i = 0; i < m_Rows * m_Columns; i++)
			m_Matrix[i] = value;
	}

	void Matrix::Destroy()
	{

	}

	Matrix Matrix::Multiply(const Matrix& B)
	{
		HZ_CORE_ASSERT(m_Columns == B.Rows()); //AB so width of A must equal Height of B
		Matrix output;
		output.Resize(m_Rows, B.Columns()); //Resize matrix A to height of A and weight of B

		for (size_t i = 0; i < m_Rows; i++)
		{
			for (size_t j = 0; j < B.Columns(); j++)
			{
				double v = 0.0;
				for (size_t k = 0; k < m_Columns; k++)
					v +=  (*this)[i][k] * B[k][j];

				output[i][j] = v;
			}
		}
		return output;
	}

	Matrix Matrix::Add(const Matrix& B)
	{
		HZ_CORE_ASSERT(this->m_Rows == B.m_Rows && this->m_Columns == B.m_Columns);
		
		for (size_t i = 0; i < m_Rows; i++)
		{
			for (size_t j = 0; j < m_Columns; j++)
			{
				(*this)[i][j] += B[i][j];
			}
		}
		return *this;
	}

	Matrix& Matrix::Scale(double scale)
	{
		for (size_t i = 0; i < m_Rows; i++)
		{
			for (size_t j = 0; j < m_Columns; j++)
			{
				(*this)[i][j] *= scale;
			}
		}
		return *this;
	}

	void Matrix::Negate()
	{
		for (size_t i = 0; i < m_Rows; i++)
		{
			for (size_t j = 0; j < m_Columns; j++)
			{
				(*this)[i][j] = -(*this)[i][j];
			}
		}
	}
}