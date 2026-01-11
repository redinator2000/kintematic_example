#ifndef KINT_RATIONAL_HPP
#define KINT_RATIONAL_HPP

#include "i2d.hpp"
#include <compare>
#include <numeric>
#include <cmath>

namespace kint
{
struct Rational
{
    using itype = i2d::ntype;
    itype num;
    itype den; // always positive

    constexpr Rational() : num(0), den(1) {}
    constexpr Rational(itype i) : num(i), den(1) {}
    constexpr Rational(itype n_num, itype n_den) :
        num(n_num), den(n_den)
    {
        if(den < 0)
        {
            num = -num;
            den = -den;
        }
    }
    auto operator<=>(const Rational & other) const
    {
        itype lhs = this->num * other.den;
        itype rhs = other.num * this->den;
        if (lhs < rhs) return std::strong_ordering::less;
        if (lhs > rhs) return std::strong_ordering::greater;
        return std::strong_ordering::equal;
    }

    constexpr Rational operator+() const
    {
        return *this;
    }
    constexpr Rational operator-() const
    {
        return Rational{-num, den};
    }
    constexpr Rational& operator+=(const Rational& other)
    {
        num = num * other.den + other.num * den;
        den = den * other.den;
        return *this;
    }

    constexpr Rational& operator-=(const Rational& other)
    {
        num = num * other.den - other.num * den;
        den = den * other.den;
        return *this;
    }
    constexpr Rational& operator*=(const Rational& other)
    {
        num = num * other.num;
        den = den * other.den;
        return *this;
    }
    constexpr Rational& operator/=(const Rational& other)
    {
        num = num * other.den;
        den = den * other.num;
        return *this;
    }
    friend constexpr Rational operator+(Rational a, const Rational& b) noexcept
    {
        a += b;
        return a;
    }
    friend constexpr Rational operator-(Rational a, const Rational& b) noexcept
    {
        a -= b;
        return a;
    }
    friend constexpr Rational operator*(Rational a, const Rational& b) noexcept
    {
        a *= b;
        return a;
    }
    friend constexpr Rational operator/(Rational a, const Rational& b) noexcept
    {
        a /= b;
        return a;
    }

    constexpr Rational reduced() const
    {
        if(num == 0)
            return Rational{0, 1};
        itype g = std::gcd(num, den);
        return Rational{num / g, den / g};
    }

    float to_float() const
    {
        return float(num) / float(den);
    }
};
constexpr bool operator==(const Rational & a, const Rational & b)
{
    if(a.num == b.num && a.den == b.den)
        return true;
    if(a.num == b.num && a.num == 0)
        return true;
    Rational ar = a.reduced();
    Rational br = b.reduced();
    return (ar.num == br.num && ar.den == br.den);
}
constexpr bool operator<(const Rational & a, const Rational & b)
{
    return a.num * b.den < b.num * a.den;
}
constexpr Rational abs(const Rational& r)
{
    return Rational{std::abs(r.num), r.den};
}
constexpr Rational::itype trunc(const Rational& r)
{
    return r.num / r.den;
}
constexpr Rational::itype floor(const Rational& r)
{
    if (r.num >= 0)
        return r.num / r.den;          // truncation is correct
    else
        return - ((-r.num + r.den - 1) / r.den);
}
constexpr Rational::itype ceil(const Rational& r)
{
    if (r.num >= 0)
        return (r.num + r.den - 1) / r.den;
    else
        return r.num / r.den;          // truncation is correct
}
constexpr Rational::itype round(const Rational& r)
{
    // r = num / den
    // compute: floor( (num + den/2) / den ) for positive
    //          ceil(  (num - den/2) / den ) for negative

    if (r.num >= 0)
        return (r.num + r.den / 2) / r.den;
    else
        return (r.num - r.den / 2) / r.den;
}
constexpr Rational fract(const Rational& r)
{
    // floor_r = floor(r)
    Rational::itype f = (r.num >= 0)
        ? (r.num / r.den)
        : - ((-r.num + r.den - 1) / r.den);

    // fractional part = r - floor(r)
    Rational result{ r.num - f * r.den, r.den };

    // result is guaranteed to be in [0, 1)
    return result;
}
Rational Rational_from_float_exact(float f);

struct Rational2D : public gcf::genericV2D_r<Rational2D, Rational>
{
    using gV2Dr = gcf::genericV2D_r<Rational2D, Rational>;
    constexpr Rational2D(){};
    constexpr Rational2D(Rational nx, Rational ny) :
        gV2Dr(nx, ny)
    {}
    constexpr Rational2D(gV2Dr g) :
        gV2Dr(g)
    {}
    constexpr Rational2D(i2d g) :
        gV2Dr(Rational{g.x}, Rational{g.y})
    {}
    constexpr Rational2D reduced() const
    {
        return Rational2D{x.reduced(), y.reduced()};
    }
};
constexpr i2d trunc(const Rational2D& r)
{
    return i2d{trunc(r.x), trunc(r.y)};
}

} // namespace kint

#endif // KINT_RATIONAL_HPP
