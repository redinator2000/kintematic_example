#include "rational.hpp"

namespace kint
{
Rational Rational_from_float_exact(float f)
{
    using itype = Rational::itype;

    uint32_t bits = std::bit_cast<uint32_t>(f);

    uint32_t sign = bits >> 31;
    uint32_t exp  = (bits >> 23) & 0xFF;
    uint32_t frac = bits & 0x7FFFFF;

    // Handle zero
    if(exp == 0 && frac == 0)
        return Rational{0};

    itype num;
    itype den = 1;

    if(exp == 0)
    {
        // Denormalized
        num = frac;
        int shift = 149; // 23 + 126
        den = itype{1} << shift;
    }
    else
    {
        // Normalized
        num = (1u << 23) | frac; // implicit leading 1
        int e = int(exp) - 127 - 23;

        if(e >= 0)
            num <<= e;
        else
            den <<= -e;
    }

    if(sign)
        num = -num;

    return Rational{num, den}.reduced();
}
} // namespace kint
