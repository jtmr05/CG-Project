#ifndef MATRIX_HPP
#define MATRIX_HPP


#include "point.hpp"


#include <array>

/** Matrix **/

template<class T, std::size_t rows, std::size_t columns>
class Matrix {

private:
    std::array<std::array<T, columns>, rows> m;

public:
    Matrix(std::initializer_list<std::array<T, columns>> list) : m(){

        assert(list.size() <= rows);

        auto iter { list.begin() };
        for(size_t i{}; iter != list.end(); ++i, ++iter)
            this->m[i] = *iter;
    }

    constexpr std::array<T, columns>& operator[](size_t i){
        return this->m[i];
    }

    constexpr const T& at(size_t i, size_t j) const {

        if(i >= rows)
            std::__throw_out_of_range_fmt(
                "Matrix::at: i (which is %zu) >= rows (which is %zu)",
                i,
                rows
            );

        if(j >= columns)
            std::__throw_out_of_range_fmt(
                "Matrix::at: j (which is %zu) >= columns (which is %zu)",
                j,
                columns
            );

        return this->m[i][j];
    }

    Matrix<T, columns, rows> transpose() const {

        Matrix<T, columns, rows> res {};

        for(size_t i {}; i < rows; ++i)
            for(size_t j {}; j < columns; ++j)
                res.m[j][i] = this->m[i][j];

        return res;
    }
};



template<size_t left_size, size_t inner_size, size_t right_size>
Matrix<double, left_size, right_size>
operator*(const Matrix<double, left_size, inner_size>  &m1,
          const Matrix<double, inner_size, right_size> &m2){

    Matrix<double, left_size, right_size> res {};

    for(size_t l {}; l < left_size; ++l){
        for(size_t r {}; r < right_size; ++r){

            double value {};

            for(size_t i {}; i < inner_size; ++i)
                value += m1.at(l, i) * m2.at(i, r);

            res[l][r] = value;
        }
    }

    return res;
}

template<size_t left_size, size_t inner_size, size_t right_size>
Matrix<CartPoint3d, left_size, right_size>
operator*(const Matrix<CartPoint3d, left_size, inner_size>  &m1,
          const Matrix<double, inner_size, right_size> &m2){

    Matrix<CartPoint3d, left_size, right_size> res {};

    for(size_t l {}; l < left_size; ++l){
        for(size_t r {}; r < right_size; ++r){

            CartPoint3d value {};

            for(size_t i {}; i < inner_size; ++i)
                value += m1.at(l, i) * m2.at(i, r);

            res[l][r] = value;
        }
    }

    return res;
}

template<size_t left_size, size_t inner_size, size_t right_size>
Matrix<CartPoint3d, left_size, right_size>
operator*(const Matrix<double, left_size, inner_size> &m1,
          const Matrix<CartPoint3d, inner_size, right_size> &m2){

    Matrix<CartPoint3d, left_size, right_size> res {};

    for(size_t l {}; l < left_size; ++l){
        for(size_t r {}; r < right_size; ++r){

            CartPoint3d value {};

            for(size_t i {}; i < inner_size; ++i)
                value += m1.at(l, i) * m2.at(i, r);

            res[l][r] = value;
        }
    }

    return res;
}

#endif