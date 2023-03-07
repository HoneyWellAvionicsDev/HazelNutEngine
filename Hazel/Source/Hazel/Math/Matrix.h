#pragma once

#include <iostream> //temp for debug
#include <iomanip>
#include <vector>
#include <type_traits>
#include <intrin.h>

namespace Jbonk::Math
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
		FORCEINLINE double* operator[](size_t row) const                { JB_CORE_ASSERT(row < m_Rows);                       return row * m_Columns + m_Matrix.get(); }
		FORCEINLINE double& operator()(size_t row, size_t column) const { JB_CORE_ASSERT(row < m_Rows || column < m_Columns); return m_Matrix[row * m_Columns + column]; }

		Matrix& operator=(const Matrix& other)
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
		//TODO: all of this is so stupid, I dont like operator overloading anymore so lets just remove this later
		Matrix operator*(Matrix& B) { return Times(B); }
		Matrix operator*(double scale) { return Scale(*this, scale); }
		Matrix operator+(Matrix& B) { return Add(B); }
		Matrix operator-(Matrix& B) { return Add(-B); }
		Matrix operator-() { return Negate(); } 
		Matrix& operator+=(const Matrix& B) { return AddToThis(B); }
		Matrix& operator-=(const Matrix& B) { return AddMinusToThis(B); }
		Matrix& operator*=(const Matrix& B) { return MultiplyToThis(B); }
		Matrix& operator*=(double scale) { return MultiplyToThis(scale); }


		//Debug methods
		void Print() 
		{
			int round = 3;
			int width = 8;
			const double pow10 = std::pow(10.0, round);
			std::cout << "----------------------------------------------\n";
			for (size_t i = 0; i < m_Rows; i++)
			{
				for (size_t j = 0; j < m_Columns; j++)
				{
					std::cout << std::setprecision(round) << std::fixed << std::setw(width) << std::right << (std::round((*this)[i][j] * pow10) / pow10) << " ";
				}
				std::cout << '\n';
			}
			std::cout << "----------------------------------------------\n";
		}
	
		//Matrix operations
		Matrix ScaleRightDiagonal(const Matrix& vector);
		Matrix ScaleLeftDiagonal(const Matrix& vector);
		Matrix Transpose();
		Matrix TransposeMultiply(const Matrix& matrix);
		Matrix Times(const Matrix& B);
		Matrix& MultiplyToThis(const Matrix& B);
		Matrix& MultiplyToThis(double scale);

		//Vector operations
		double Magnitude() const;
		double MagnitudeSquared() const;
		double Dot(const Matrix& vector) const;

		//Matrix and vector
		Matrix Add(const Matrix& B);
		Matrix& AddToThis(const Matrix& B);
		Matrix& AddMinusToThis(const Matrix& B);
		Matrix Scale(const Matrix& B, double scale);
		Matrix Negate();

		//static methods
		static void Multiply(const Matrix& A, const Matrix& B, Matrix& C)
		{
			JB_CORE_ASSERT(A.m_Columns == B.m_Rows);

			C.Resize(A.m_Rows, B.m_Columns);

			for (size_t i = 0; i < A.m_Rows; i++)
			{
				for (size_t j = 0; j < B.m_Columns; j++)
				{
					double v = 0.0;
					for (size_t k = 0; k < A.m_Columns; k++)
						v += A[i][k] * B[k][j];

					C[i][j] = v;
				}
			}
		}

		static void Multiply(const Matrix& A, double scale, Matrix& C)
		{
			for (size_t i = 0; i < A.m_Rows; i++)
			{
				for (size_t j = 0; j < A.m_Columns; j++)
				{
					C[i][j] = scale * A[i][j];
				}
			}
		}

		static void ScaleLeftDiagonal(const Matrix& A, const Matrix& diagonal, Matrix& C)
		{
			JB_CORE_ASSERT(diagonal.m_Columns < 2);
			JB_CORE_ASSERT(diagonal.m_Rows == A.m_Rows);

			C.Resize(A.m_Rows, A.m_Columns);

			for (size_t i = 0; i < A.m_Rows; i++)
			{
				for (size_t j = 0; j < A.m_Columns; j++)
				{
					C[i][j] = diagonal[i][0] * A[i][j];
				}
			}
		}

	private:
		friend Matrix operator* (double scale, Matrix& mat);
		size_t m_Rows, m_Columns;
		Scope<double[]> m_Matrix;
	};

	inline Matrix operator*(double scale, Matrix& mat) { return mat.Scale(mat, scale); } //since this definition is inside a header file it needs to be marked as inline
	
	template<typename T>
	concept FloatType = std::is_same_v<T, float> || std::is_same_v<T, double>;

	template<FloatType T>
	class SparseMatrix
	{
	public:
		SparseMatrix(uint16_t rows, uint16_t columns);
		T get(size_t row, size_t column) const;
		void set(size_t row, size_t column, const T& value);
		void update_row_indices(size_t row);
	private:
		uint16_t _rows, _columns;

		std::vector<T> _nnz_values;
		std::vector<size_t> _col_index;
		std::vector<size_t> _row_index;

	};

	template<FloatType T>
	inline SparseMatrix<T>::SparseMatrix(uint16_t rows, uint16_t columns)
		: _rows(rows), _columns(columns)
	{
		_row_index = std::vector<size_t>(rows + 1, 0);
	}

	template<FloatType T>
	inline void SparseMatrix<T>::update_row_indices(size_t row)
	{
		if (row == _rows - 1)
			_row_index[row + 1]++;

		for (size_t i = _row_index[row]; i < _row_index.size(); i++)
			_row_index[i]++;
	}

	template<FloatType T>
	inline void SparseMatrix<T>::set(size_t row, size_t column, const T& value)
	{
		if (value == T())
			return;

		if (_nnz_values.empty())
			return;

		size_t row_start = _row_index[row];
		size_t row_end = _row_index[row + 1] - 1;

		if (_col_index[row_start] == column)
		{
			_nnz_values[row_start] = value;
			return;
		}

		if (_col_index[row_end] < column)
		{
			_nnz_values.insert(_nnz_values.begin() + row_end + 1, value);
			_col_index.insert(_col_index.begin() + row_end + 1, column);
			update_row_indices(row);
			return;
		}

		if (_col_index[row_start] > column)
		{
			_nnz_values.insert(_nnz_values.begin() + row_start, value);
			_col_index.insert(_col_index.begin() + row_start, column);
			update_row_indices(row);
			return;
		}
		
		for (size_t i = _col_index[row_start]; i < _col_index[row_end] + 1; i++)
		{
			if (i == column)
			{
				_nnz_values.insert(_nnz_values.begin() + i + 2, value);
				_col_index.insert(_col_index.begin() + i + 2, column);
				update_row_indices(row);
				return;
			}
		}
	}


	template<FloatType T>
	inline T SparseMatrix<T>::get(size_t row, size_t column) const
	{
		size_t row_start = _row_index[row];
		size_t row_end = _row_index[row + 1];
		size_t current_col = 0;
		
		for (size_t i = row_start; i < row_end; i++)
		{
			current_col = _col_index[i];

			if (current_col > column)
				break;

			if (current_col == column)
				return _nnz_values[i];
		}
		 
		return T();
	}
}
