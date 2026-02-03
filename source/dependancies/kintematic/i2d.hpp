#ifndef KINT_V2D_HPP
#define KINT_V2D_HPP

#include "gridcoarsefine.hpp"
#include <cstdint>

namespace kint
{
using i2d = gcf::intV2D<int64_t, 1>;
constexpr bool axis_aligned(const i2d & v)
{
    return v.x == 0 || v.y == 0;
}
}

#endif // KINT_V2D_HPP
