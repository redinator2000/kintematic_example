#ifndef GRIDCOARSEFINE_HPP
#define GRIDCOARSEFINE_HPP

#include <concepts>

namespace gcf
{

template<typename R, typename T, T tag = 0>
struct genericV2D_r
{
    using ntype = T;

    ntype x;
    ntype y;

    constexpr genericV2D_r() noexcept = default;

    constexpr genericV2D_r(ntype nx, ntype ny) noexcept
    : x(nx), y(ny)
    {}

    [[nodiscard]] constexpr R operator+() const noexcept
    {
        return *this;
    }

    [[nodiscard]] constexpr R operator-() const noexcept
    {
        return genericV2D_r(-x, -y);
    }

    constexpr R operator+=(const genericV2D_r& b) noexcept
    {
        x += b.x;
        y += b.y;
        return *this;
    }
    constexpr R operator-=(const genericV2D_r& b) noexcept
    {
        x -= b.x;
        y -= b.y;
        return *this;
    }
    constexpr R operator*=(const ntype& b) noexcept
    {
        x *= b;
        y *= b;
        return *this;
    }
    constexpr R operator/=(const ntype& b) noexcept
    {
        x /= b;
        y /= b;
        return *this;
    }

    bool operator==(const genericV2D_r&) const noexcept = default;

    friend constexpr R operator+(genericV2D_r a, const genericV2D_r& b) noexcept
    {
        a += b;
        return a;
    }
    friend constexpr R operator-(genericV2D_r a, const genericV2D_r& b) noexcept
    {
        a -= b;
        return a;
    }
    friend constexpr R operator*(genericV2D_r a, const ntype& b) noexcept
    {
        a *= b;
        return a;
    }
    friend constexpr R operator*(const ntype& b, genericV2D_r a) noexcept
    {
        a *= b;
        return a;
    }
    friend constexpr R operator/(genericV2D_r a, const ntype& b) noexcept
    {
        a /= b;
        return a;
    }
};
template<typename T, typename R, T tag>
T cross(const genericV2D_r<R, T, tag> & a, const genericV2D_r<R, T, tag> & b)
{
    return a.x * b.y - a.y * b.x;
};
template<typename T, typename R, T tag>
T dot(const genericV2D_r<R, T, tag> & a, const genericV2D_r<R, T, tag> & b)
{
    return a.x * b.x + a.y * b.y;
};
template<typename T, T tag = 0>
struct genericV2D final : genericV2D_r<genericV2D<T, tag>, T, tag>
{
    using gV2Dr = genericV2D_r<genericV2D<T, tag>, T, tag>;
    constexpr genericV2D(){};
    constexpr genericV2D(T nx, T ny) :
        gV2Dr(nx, ny)
    {}
    constexpr genericV2D(gV2Dr g) :
        gV2Dr(g)
    {}
    template<typename R>
    constexpr operator genericV2D_r<R, T, tag>() const
    {
        return genericV2D_r<R, T, tag>{this->x, this->y};
    }
};

template <std::integral Tint>
constexpr bool power_of_two(Tint b)
{
    return (b > 0 && (b & (b - 1)) == 0);
}

template <std::integral Tint>
constexpr Tint modPos(Tint a, Tint b)
{
    Tint r = a % b;
    return r < 0 ? r + b : r;
}
template <auto b, std::integral Tint>
    requires(power_of_two(b))
constexpr Tint modPos_binary(Tint a)
{
    Tint out = a & (b - 1);
    //assert(out == modPos(a, b));
    return out;
}
template <std::integral Tint>
constexpr Tint divPos(Tint a, Tint b)
{
    if (a >= 0)
        return a / b;
    return a / b - (a % b == 0 ? 0 : 1);
}
template <auto b, std::integral Tint>
    requires(power_of_two(b))
constexpr Tint divPos_binary(Tint a)
{
    Tint out = (a & ~(b - 1)) / b;
    //assert(out == divPos(a, b));
    return out;
}

template <std::integral Tint, Tint es> //es is the courseness of a grid, with 1 being the tiniest squares
struct intV2D : public genericV2D_r<intV2D<Tint, es>, Tint, es>
{
    using gV2Dr = genericV2D_r<intV2D<Tint, es>, Tint, es>;
    using itype = gV2Dr::ntype;
    static constexpr Tint es_value = es;

    constexpr intV2D(){};
    constexpr intV2D(Tint nx, Tint ny) :
        gV2Dr(nx, ny)
    {}
    constexpr intV2D(gV2Dr g) :
        gV2Dr(g)
    {}

    template <Tint oes>
        requires(oes != es &&
                 power_of_two(oes) && power_of_two(es))
    constexpr explicit intV2D(const intV2D<Tint, oes> & v)
    {
        if(oes > es)
        {
            this->x = v.x * oes / es;
            this->y = v.y * oes / es;
        }
        else
        {
            this->x = divPos_binary<es>(v.x * oes);
            this->y = divPos_binary<es>(v.y * oes);
        }
    }
    template <Tint oes>
        requires(oes != es &&
                 !(power_of_two(oes) && power_of_two(es)))
    constexpr explicit intV2D(const intV2D<Tint, oes> & v)
    {
        if(oes > es)
        {
            this->x = v.x * oes / es;
            this->y = v.y * oes / es;
        }
        else
        {
            this->x = divPos(v.x * oes, es);
            this->y = divPos(v.y * oes, es);
        }
    }
    template <std::integral oTint>
        requires(sizeof(oTint) < sizeof(Tint)) //can only cast to bigger size
    constexpr intV2D(const intV2D<oTint, oTint(es)> & v)
    {
        this->x = v.x;
        this->y = v.y;
    }
    template <std::integral oTint>
    static constexpr intV2D trim_cast(const intV2D<oTint, es> & v)
    {
        return intV2D(v.x, v.y);
    }
    template <std::integral oTint, oTint oes>
        requires(oes != es &&
                 power_of_two(oes) &&
                 sizeof(oTint) < sizeof(Tint))
    constexpr explicit intV2D(const intV2D<oTint, oes> & v)
    {
        auto bigger_size = intV2D<Tint, Tint(oes)>(v);
        *this = intV2D<Tint, es>(bigger_size);
    }
    template <std::integral oTint, oTint oes>
        requires(oes != es &&
                 power_of_two(oes) &&
                 sizeof(oTint) > sizeof(Tint))
    constexpr explicit intV2D(const intV2D<oTint, oes> & v)
    {
        static_assert(oes * 16 <= es); // a fine int64 grid can be cast to a course int32 grid. Use trim_cast for a more explicit conversion
        auto shifted = intV2D<oTint, oTint(es)>(v);
        *this = trim_cast<oTint>(shifted);
    }

    // bool operator== (const intV2D &) const = default;
};
}

/* example usage:

#include <cstdint>
#include <unordered_map>
using namespace gcf;

const int chunk_size = 64;

using pixelV2D = intV2D<int64_t, 16>;
using tileV2D = intV2D<int32_t, 16 * 16>;
using chunkV2D = intV2D<int32_t, 16 * 16 * chunk_size>;

struct tile {};
struct Chunk
{
    tile tiles[chunk_size][chunk_size];
};

const Chunk * get_chunk(chunkV2D); //maybe an unordered_map with chunkV2D key

tile get_tile_at_pixel(pixelV2D pixel_grid_position)
{
    tileV2D tile_grid_position = tileV2D(pixel_grid_position); //these conversions will use divPos_binary, which compiles to a bit shift
    chunkV2D chunk_grid_position = chunkV2D(tile_grid_position);
    const Chunk * chunk = get_chunk(chunk_grid_position);

    tileV2D tile_within_chunk = tileV2D(modPos_binary<chunk_size>(tile_grid_position.x),
                                        modPos_binary<chunk_size>(tile_grid_position.y)); //modPos_binary is just a binary &
    return chunk->tiles[tile_within_chunk.y][tile_within_chunk.x];
}
*/

#endif // GRIDCOARSEFINE_HPP
