#ifndef KINT_ADVANCE_HPP
#define KINT_ADVANCE_HPP

#include "shape.hpp"
#include <optional>

namespace kint
{
struct Impact;
struct Minkowski_Set;
std::vector<Impact> impact_occlusion_filter(std::span<const Impact>);
std::vector<Impact> move_and_slide(Shape_Rectangle &, const Minkowski_Set &, i2d::ntype max_escape_distance = 32, std::optional<i2d> step_vector = std::nullopt);
// step vector should be the axis-aligned up direction with length of max step height
}

#endif // KINT_ADVANCE_HPP
