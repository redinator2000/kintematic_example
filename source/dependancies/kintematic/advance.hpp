#ifndef KINT_ADVANCE_HPP
#define KINT_ADVANCE_HPP

#include "shape.hpp"

namespace kint
{
struct Impact;
struct Minkowski_Set;
i2d clip_and_slide(Shape_Point, const Minkowski_Set &);
std::vector<Impact> impact_occlusion_filter(std::span<const Impact>);
}

#endif // KINT_ADVANCE_HPP
