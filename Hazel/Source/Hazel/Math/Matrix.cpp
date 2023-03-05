#include "hzpch.h"
#include "Matrix.h"

namespace Jbonk::Math
{
	Matrix::Matrix()
		: m_Matrix(nullptr), m_Rows(0), m_Columns(0)
	{
	}

	Matrix::Matrix(size_t rows, size_t columns, double value)
		: m_Matrix(nullptr), m_Rows(rows), m_Columns(columns)
	{
		Initialize(rows, columns, value);
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

	void Matrix::InsertMatrix(size_t row, size_t column, const Matrix& subMatrix)
	{
		JB_CORE_ASSERT(row < m_Rows && column < m_Columns);

		for (size_t i = 0; i < subMatrix.m_Rows; i++)
		{
			for (size_t j = 0; j < subMatrix.m_Columns; j++)
			{
				(*this)[i + row][j + column] = subMatrix[i][j];
			}
		}
	}

	Matrix Matrix::ScaleRightDiagonal(const Matrix& vector)
	{
		JB_CORE_ASSERT(vector.m_Columns < 2);
		JB_CORE_ASSERT(vector.m_Rows == this->m_Columns);

		Matrix output = *this;

		for (size_t i = 0; i < m_Rows; i++)
		{
			for (size_t j = 0; j < m_Columns; j++)
			{
				output[i][j] *= vector[j][0];
			}
		}

		return output;
	}

	Matrix Matrix::ScaleLeftDiagonal(const Matrix& vector)
	{
		JB_CORE_ASSERT(vector.m_Columns < 2);
		JB_CORE_ASSERT(vector.m_Rows == this->m_Rows);

		Matrix output = *this;

		for (size_t i = 0; i < m_Rows; i++)
		{
			for (size_t j = 0; j < m_Columns; j++)
			{
				output[i][j] *= vector[i][0];
			}
		}

		return output;
	}

	Matrix Matrix::TransposeMultiply(const Matrix& B)
	{
		JB_CORE_ASSERT(m_Rows == B.Rows()); //AB so width of A must equal Height of B
		Matrix output(m_Rows, B.m_Columns);

		for (size_t i = 0; i < m_Rows; i++)
		{
			for (size_t j = 0; j < B.m_Columns; j++)
			{
				double v = 0.0;
				for (size_t k = 0; k < m_Columns; k++)
					v += (*this)[k][i] * B[k][j];

				output[i][j] = v;
			}
		}
		return output;
	}

	Matrix Matrix::Transpose()
	{
		Matrix output(m_Columns, m_Rows);

		for (size_t i = 0; i < m_Rows; i++)
		{
			for (size_t j = 0; j < m_Columns; j++)
			{
				output[j][i] = (*this)[i][j];
			}
		}
		return output;
	}

	Matrix Matrix::Times(const Matrix& B) 
	{
		JB_CORE_ASSERT(m_Columns == B.Rows()); //AB so width of A must equal Height of B
		Matrix output(m_Rows, B.m_Columns);

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

	Matrix& Matrix::MultiplyToThis(const Matrix& B)
	{
		JB_CORE_ASSERT(m_Columns == B.Rows()); //AB so width of A must equal Height of B

		Resize(m_Rows, B.Columns());

		for (size_t i = 0; i < m_Rows; i++)
		{
			for (size_t j = 0; j < B.Columns(); j++)
			{
				double v = 0.0;
				for (size_t k = 0; k < m_Columns; k++)
					v += (*this)[i][k] * B[k][j];

				(*this)[i][j] = v;
			}
		}
		return *this;
	}

	Matrix& Matrix::MultiplyToThis(double scale)
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

	double Matrix::Magnitude() const
	{
		JB_CORE_ASSERT(this->m_Columns == 1);

		double mag = 0.0;
		for (size_t i = 0; i < m_Rows; i++)
		{
			mag += (*this)[i][0] * (*this)[i][0];
		}

		return glm::sqrt(mag);
	}

	//returns the magnituge squared
	double Matrix::MagnitudeSquared() const
	{
		JB_CORE_ASSERT(this->m_Columns == 1);

		double mag = 0.0;
		for (size_t i = 0; i < m_Rows; i++)
		{
			mag += (*this)[i][0] * (*this)[i][0];
		}
		return mag;
	}

	double Matrix::Dot(const Matrix& b) const
	{
		JB_CORE_ASSERT(this->m_Columns == 1);
		JB_CORE_ASSERT(this->m_Columns == b.m_Columns);
		JB_CORE_ASSERT(this->m_Rows == b.m_Rows);

		double dot = 0.0;
		for (size_t i = 0; i < m_Rows; i++)
		{
			dot += (*this)[i][0] * b[i][0];
		}
		return dot;
	}

	Matrix Matrix::Add(const Matrix& B) 
	{
		JB_CORE_ASSERT(this->m_Rows == B.m_Rows && this->m_Columns == B.m_Columns);
		
		Matrix output = *this;

		for (size_t i = 0; i < m_Rows; i++)
		{
			for (size_t j = 0; j < m_Columns; j++)
			{
				output[i][j] += B[i][j];
			}
		}
		return output;
	}

	Matrix& Matrix::AddToThis(const Matrix& B)
	{
		JB_CORE_ASSERT(this->m_Rows == B.m_Rows && this->m_Columns == B.m_Columns);

		for (size_t i = 0; i < m_Rows; i++)
		{
			for (size_t j = 0; j < m_Columns; j++)
			{
				(*this)[i][j] += B[i][j];
			}
		}
		return *this;
	}

	Matrix& Matrix::AddMinusToThis(const Matrix& B)
	{
		JB_CORE_ASSERT(this->m_Rows == B.m_Rows && this->m_Columns == B.m_Columns);

		for (size_t i = 0; i < m_Rows; i++)
		{
			for (size_t j = 0; j < m_Columns; j++)
			{
				(*this)[i][j] -= B[i][j];
			}
		}
		return *this;
	}

	Matrix Matrix::Scale(const Matrix& B, double scale)
	{
		Matrix scaled(m_Rows, m_Columns);
		for (size_t i = 0; i < m_Rows; i++)
		{
			for (size_t j = 0; j < m_Columns; j++)
			{
				scaled[i][j] = scale * B[i][j];
			}
		}
		return scaled;
	}

	Matrix Matrix::Negate()
	{
		Matrix negated(m_Rows, m_Columns);
		for (size_t i = 0; i < m_Rows; i++)
		{
			for (size_t j = 0; j < m_Columns; j++)
			{
				negated[i][j] = -(*this)[i][j];
			}
		}
		return negated;
	}
}
