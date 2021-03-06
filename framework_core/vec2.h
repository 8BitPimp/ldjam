#pragma once
#include <cmath>
#include <cstdint>

namespace tengu {
namespace {
float isqrt(const float& val)
{
    const float threehalfs = 1.5f;
    float x2 = val * 0.5f;
    float y = val;
    long i = *(long*)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*)&i;
    y = y * (threehalfs - (x2 * y * y));
    return y;
}

float hermite(float y0, float y1, float y2, float y3, float x)
{
    float c0 = y1;
    float c1 = 0.5f * (y2 - y0);
    float c3 = 1.5f * (y1 - y2) + 0.5f * (y3 - y0);
    float c2 = y0 - y1 + c1 - c3;
    return ((c3 * x + c2) * x + c1) * x + c0;
}
} // namespace {}

template <typename type_t>
struct vec2_t {
    type_t x, y;

    vec2_t() = default;

    template <typename other_t>
    explicit vec2_t(const vec2_t<other_t>& other)
        : x(type_t(other.x))
        , y(type_t(other.y))
    {
    }

    template <typename input_t>
    vec2_t(input_t a_x, input_t a_y)
        : x(type_t(a_x))
        , y(type_t(a_y))
    {
    }

    static vec2_t nearest(const vec2_t& origin,
        const vec2_t& a,
        const vec2_t& b)
    {
        if (distance_sqr(origin, a) < distance_sqr(origin, b)) {
            return a;
        } else {
            return b;
        }
    }

    static float sqrt(const float& x)
    {
        return sqrtf(x);
    }

    void operator+=(const vec2_t& v)
    {
        x += v.x;
        y += v.y;
    }

    void operator-=(const vec2_t& v)
    {
        x -= v.x;
        y -= v.y;
    }

    void operator*=(const type_t v)
    {
        x *= v;
        y *= v;
    }

    void operator/=(const type_t v)
    {
        x /= v;
        y /= v;
    }

    template <typename other_t>
    static vec2_t cast(const vec2_t<other_t>& a)
    {
        return vec2_t{
            type_t(a.x),
            type_t(a.y)
        };
    }

    static type_t length(const vec2_t& v)
    {
        return ::sqrt(v.x * v.x + v.y * v.y);
    }

    static vec2_t normalize(const vec2_t& v)
    {
        const type_t l = length(v);
        return vec2_t{
            v.x / l,
            v.y / l,
        };
    }

    static vec2_t cross(const vec2_t& a)
    {
        return vec2_t{ a.y, -a.x };
    }

    static type_t distance(
        const vec2_t& a,
        const vec2_t& b)
    {
        return length(b - a);
    }

    static type_t distance_sqr(
        const vec2_t& a,
        const vec2_t& b)
    {
        return (b - a) * (b - a);
    }

    static vec2_t zero()
    {
        return vec2_t{
            0, 0
        };
    }

    // project v onto u
    static vec2_t project(
        const vec2_t& u,
        const vec2_t& v)
    {
        const type_t vu = v * u;
        const type_t uu = u * u;
        return u * (vu / uu);
    }

    static vec2_t rotate(
        const vec2_t& v,
        const type_t angle)
    {
        const type_t s = sinf(angle);
        const type_t c = cosf(angle);
        return vec2_t{
            c * v.x + s * v.y,
            c * v.y - s * v.x
        };
    }

    static vec2_t lerp(
        const vec2_t& a,
        const vec2_t& b,
        const type_t i)
    {
        return a + (b - a) * i;
    }

    static vec2_t hlerp(
        const vec2_t& pm,
        const vec2_t& p0,
        const vec2_t& p1,
        const vec2_t& p2,
        const type_t i)
    {
        return vec2_t{ hermite(pm.x, p0.x, p1.x, p2.x, i),
            hermite(pm.y, p0.y, p1.y, p2.y, i) };
    }

    static vec2_t scale(
        const vec2_t& a,
        const vec2_t& b)
    {
        return vec2_t{ a.x * b.x, a.y * b.y };
    }
};

namespace {

template <typename type_t>
vec2_t<type_t> operator+(
    const vec2_t<type_t>& a,
    const vec2_t<type_t>& b)
{
    return vec2_t<type_t>{
        a.x + b.x,
        a.y + b.y
    };
}

template <typename type_t>
vec2_t<type_t> operator-(
    const vec2_t<type_t>& a,
    const vec2_t<type_t>& b)
{
    return vec2_t<type_t>{
        a.x - b.x,
        a.y - b.y
    };
}

template <typename type_t>
type_t operator*(
    const vec2_t<type_t>& a,
    const vec2_t<type_t>& b)
{
    return a.x * b.x + a.y * b.y;
}

template <typename type_t>
vec2_t<type_t> operator*(
    const vec2_t<type_t>& a,
    const type_t s)
{
    return vec2_t<type_t>{
        a.x * s,
        a.y * s
    };
}

template <typename type_t>
vec2_t<type_t> operator*(
    const type_t s,
    const vec2_t<type_t>& a)
{
    return vec2_t<type_t>{
        a.x * s,
        a.y * s
    };
}

template <typename type_t>
vec2_t<type_t> operator/(
    const vec2_t<type_t>& a,
    const type_t s)
{
    return vec2_t<type_t>{
        a.x / s,
        a.y / s
    };
}

template <typename type_t>
vec2_t<type_t> operator-(
    const vec2_t<type_t>& a)
{
    return vec2_t<type_t>{
        -a.x,
        -a.y
    };
}

typedef vec2_t<float> vec2f_t;
typedef vec2_t<int32_t> vec2i_t;

} // namespace {}
} // namespace tengu
