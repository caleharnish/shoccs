#pragma once

#include "types.hpp"

namespace ccs::matrix
{

// encapsulate information common to all matrix implementations
class matrix_base
{
    integer rows_;
    integer columns_;
    integer row_offset_;
    integer col_offset_;
    integer stride_;

public:
    matrix_base() = default;

    matrix_base(integer rows,
                integer columns,
                integer row_offset = 0,
                integer col_offset = 0,
                integer stride = 1)
        : rows_{rows},
          columns_{columns},
          row_offset_{row_offset},
          col_offset_{col_offset},
          stride_{stride}
    {
    }

    constexpr auto rows() const { return rows_; }
    constexpr auto columns() const { return columns_; };
    constexpr auto row_offset() const { return row_offset_; }
    constexpr auto col_offset() const { return col_offset_; }
    constexpr auto stride() const { return stride_; }
    // these can be set after initialization
    matrix_base& row_offset(integer x)
    {
        row_offset_ = x;
        return *this;
    }
    matrix_base& col_offset(integer x)
    {
        col_offset_ = x;
        return *this;
    }
    matrix_base& stride(integer x)
    {
        stride_ = x;
        return *this;
    }
};
} // namespace ccs::matrix