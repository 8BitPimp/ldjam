#include <assert.h>
#include "spatial.h"

namespace {
    float clamp(
        const float min,
        const float in,
        const float max)
    {
        const float t0 = (in < min) ? min : in;
        const float t1 = (t0 > max) ? max : t0;
        return t1;
    }
} // namespace {}

void body_pair_set_t::insert(body_t * a, body_t * b)
{
    base_.insert(pair_t(a, b));
}

void body_pair_set_t::clear()
{
    base_.clear();
}

void body_set_t::insert(body_t * a)
{
    base_.insert(a);
}

bool body_set_t::operator[] (body_t * a) const {
    return base_.find(a) != base_.cend();
}

void body_set_t::clear()
{
    base_.clear();
}

std::list<spatial_t::slot_t> & spatial_t::slot(int32_t x, int32_t y)
{
    return hash_[(x+y * (512/width))%hash_.size()];
}

spatial_t::bound_t spatial_t::object_bound(body_t * obj)
{
    return bound_t {
        int32_t((obj->x-obj->r) / width),
        int32_t((obj->y-obj->r) / width),
        int32_t((obj->x+obj->r) / width),
        int32_t((obj->y+obj->r) / width)
    };
}

void spatial_t::insert(body_t * obj)
{
    bound_t ob = object_bound(obj);
    for (int32_t y = ob.y0; y<=ob.y1; ++y) {
        for (int32_t x = ob.x0; x<=ob.x1; ++x) {
            slot(x, y).push_front(slot_t{ x, y, obj });
        }
    }
}

void spatial_t::slot_erase(int32_t x, int32_t y, body_t * obj)
{
    auto & list = slot(x, y);
    for (auto itt = list.begin(); itt!=list.end();) {
        if (itt->obj==obj && itt->x==x && itt->y==y) {
            itt = list.erase(itt);
        }
        else {
            ++itt;
        }
    }
}

void spatial_t::slot_insert(int32_t x, int32_t y, body_t * obj)
{
    std::list<slot_t> & list = slot(x, y);
    list.push_front(slot_t{x, y, obj});
}

void spatial_t::remove(body_t * obj)
{
    bound_t ob = object_bound(obj);
    for (int32_t y = ob.y0; y<=ob.y1; ++y) {
        for (int32_t x = ob.x0; x<=ob.x1; ++x) {
            slot_erase(x, y, obj);
        }
    }
}

void spatial_t::move(
    body_t * obj,
    const bound_t & ob0,
    const bound_t & ob1)
{
    // return if bounds are the same
    if (ob0 == ob1) {
        return;
    }
    // erase redundant part of old bound
    for (int32_t y = ob0.y0; y<=ob0.y1; ++y) {
        for (int32_t x = ob0.x0; x<=ob0.x1; ++x) {
            if (!ob1.contains(x, y))
                slot_erase(x, y, obj);
        }
    }
    // add additional part of new bound
    for (int32_t y = ob1.y0; y<=ob1.y1; ++y) {
        for (int32_t x = ob1.x0; x<=ob1.x1; ++x) {
            if (!ob0.contains(x, y))
                slot_insert(x, y, obj);
        }
    }
}

void spatial_t::query_collisions(body_pair_set_t & out)
{
    uint32_t compares = 0;
    // for each cell
    for (auto & cell:hash_) {
        // for each object in the cell
        for (auto a = cell.cbegin(); a!=cell.cend(); ++a) {
            // get next object
            auto b = a;
            if (b==cell.cend())
                continue;
            ++b;
            // for each pair in this cell
            for (; b!=cell.cend(); ++b) {

                compares++;

                // pairs under test
                auto oa = a->obj, ob = b->obj;
                // distance between objects
                const float dx = ob->x - oa->x;
                const float dy = ob->y - oa->y;
                const float ds = dx*dx + dy*dy;

                // ideal distance
                const float id = (oa->r + ob->r) * (oa->r + ob->r);

                // if bounds intersect
                if (ds < id) {
                    // add to collision set
                    out.insert(a->obj, b->obj);
                }
            }
        }
    }
#if 1
    printf("?> %d\n", compares);
#endif
}

void spatial_t::query_radius(
    float x,
    float y,
    float r,
    body_set_t & out)
{
#if 0
    // ideal distance
    const float id = (obj->r + r) * (obj->r + r);
#endif
}

static bool overlap(
    body_t * obj,
    float x0,
    float y0,
    float x1,
    float y1)
{
    assert(obj);
    const float cx = clamp( x0, obj->x, x1 );
    const float cy = clamp( y0, obj->y, y1 );
    const float dx = obj->x - cx;
    const float dy = obj->y - cy;
    const float ds = dx*dx + dy*dy;
    return ds < (obj->r * obj->r);
}

void spatial_t::query_rect(
    float x0,
    float y0,
    float x1,
    float y1,
    body_set_t & out)
{
    // transform bounds into hash space
    int32_t sx0 = x0, sy0 = y0;
    int32_t sx1 = x1, sy1 = y1;
    xform_in(sx0, sy0);
    xform_in(sx1, sy1);

    // hash area covered by rect
    for (int32_t iy=sy0; iy<=sy1; ++iy) {
        for (int32_t ix = sx0; ix <= sx1; ++ix) {

            std::list<slot_t> &cell = slot(ix, iy);

            for (auto a = cell.cbegin(); a != cell.cend(); ++a) {

                if (overlap(a->obj, x0, y0, x1, y1)) {

                    out.insert(a->obj);
                }
            }
        }
    }
}

void spatial_t::query_ray(
    float x1,
    float y1,
    float x2,
    float y2,
    body_set_t & out)
{

}

int spatial_t::dbg_ocupancy(int32_t x, int32_t y)
{
    std::list<slot_t> & list = slot(x, y);
    return list.size();
}
