#ifndef KINT_ADVANCE_HPP
#define KINT_ADVANCE_HPP

#include "shape.hpp"
#include <optional>

namespace kint
{
struct Impact_ID;
struct Minkowski_Set;

struct Platformer_Properties
{
    i2d down_direction;
    i2d::ntype step_height;
};

i2d clip_velocity(Shape_Point rect, const Minkowski_Set & mset, std::vector<Impact_ID> * impacts_out, bool do_slide);
std::vector<Impact_ID> move_and_slide(Shape_Rectangle &, const Minkowski_Set &, i2d::ntype max_escape_distance = 32, std::optional<Platformer_Properties> platformer_properties = std::nullopt);
// step vector should be the axis-aligned up direction with length of max step height
}

#endif // KINT_ADVANCE_HPP
