#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <functional>
#include <list>
#include <unordered_set>

#include "../framework_core/random.h"
#include "../framework_core/vec2.h"

namespace tengu {
struct body_t {
    body_t() = default;

    body_t(const vec2f_t& pos,
        const float radius,
        struct object_t* object)
        : p(pos)
        , r(radius)
        , vel_(vec2f_t{ 0.f, 0.f })
        , obj_(object)
    {
    }

    vec2f_t pos() const
    {
        return p;
    }

    float radius() const
    {
        return r;
    }

    vec2f_t vel_;

    struct object_t* obj_;

protected:
    friend struct spatial_t;
    vec2f_t p;
    float r;
};

struct body_pair_set_t {
    typedef std::pair<body_t*, body_t*> pair_t;

    struct hash_func_t {
        size_t operator()(const pair_t& in) const
        {
            return size_t(
                tengu::hash_t::wang_64(uint64_t(in.first) ^ tengu::hash_t::wang_64(uint64_t(in.second))));
        }
    };

    void insert(body_t* a, body_t* b);

    void clear();

    typedef std::unordered_set<pair_t, hash_func_t> base_t;

    base_t::iterator begin()
    {
        return base_.begin();
    }

    base_t::iterator end()
    {
        return base_.end();
    }

protected:
    base_t base_;
};

struct body_set_t {
    typedef std::unordered_set<body_t*> base_t;

    void insert(body_t* a);

    void clear();

    bool operator[](body_t* a) const;

    base_t::iterator begin()
    {
        return base_.begin();
    }

    base_t::iterator end()
    {
        return base_.end();
    }

protected:
    base_t base_;
};

struct spatial_t {
    static const uint32_t width = 32;
    static const uint32_t items = 1024;

    void insert(body_t* obj);

    void remove(const body_t* obj);

    void move(body_t* obj, const vec2f_t& pos)
    {
        assert(obj);
        bound_t ob0 = object_bound(obj);
        obj->p = pos;
        bound_t ob1 = object_bound(obj);
        move(obj, ob0, ob1);
    }

    void move(body_t* obj, const vec2f_t& pos, const float radius)
    {
        assert(obj);
        bound_t ob0 = object_bound(obj);
        obj->p = pos;
        obj->r = radius;
        bound_t ob1 = object_bound(obj);
        move(obj, ob0, ob1);
    }

    void query_collisions(body_pair_set_t& out);

    void query_radius(const vec2f_t& p, float r, body_set_t& out);

    void query_rect(const vec2f_t& p0, const vec2f_t& p1, body_set_t& out);

    void query_ray(const vec2f_t& p0, const vec2f_t& p1, body_set_t& out);

    int dbg_ocupancy(int32_t x, int32_t y);

protected:
    struct slot_t {
        int32_t x, y;
        body_t* obj;
    };

    struct bound_t {
        int x0, y0;
        int x1, y1;

        bool contains(int32_t x, int32_t y) const
        {
            return x >= x0 && x <= x1 && y >= y0 && y <= y1;
        }

        bool operator==(const bound_t& o) const
        {
            return memcmp(this, &o, sizeof(o)) == 0;
        }
    };

    std::array<std::list<slot_t>, items> hash_;

    std::list<slot_t>& slot(int32_t x, int32_t y);

    bound_t object_bound(const body_t* obj) const;

    void slot_test(int32_t x, int32_t y, float r, body_set_t& out);

    void slot_erase(int32_t x, int32_t y, const body_t* obj);

    void slot_insert(int32_t x, int32_t y, body_t* obj);

    void move(body_t*, const bound_t&, const bound_t&);

    void xform_in(int32_t& x, int32_t& y)
    {
        x /= width;
        y /= width;
    }
};

struct body_ex_t {

    body_t body_;

    body_ex_t(spatial_t& spatial,
        const vec2f_t& pos,
        float radius,
        struct object_t* obj)
        : spatial_(spatial)
        , body_(pos, radius, obj)
    {
        spatial_.insert(&body_);
    }

    body_ex_t(const body_ex_t& other)
        : spatial_(other.spatial_)
        , body_(other.body_)
    {
        spatial_.insert(&body_);
    }

    void operator=(const body_ex_t&) = delete;

    ~body_ex_t()
    {
        spatial_.remove(&body_);
    }

    void move(const vec2f_t& p)
    {
        spatial_.move(&body_, p);
    }

    const vec2f_t pos() const
    {
        return body_.pos();
    }

    vec2f_t& vel()
    {
        return body_.vel_;
    }

protected:
    spatial_t& spatial_;
};
} // namespace tengu
