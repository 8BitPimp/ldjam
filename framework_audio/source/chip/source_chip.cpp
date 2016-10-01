#include <cassert>
#include "source_chip.h"

namespace {
template <typename type_t>
constexpr type_t _min(type_t a, type_t b) {
    return (a<b) ? a : b;
}

template <typename type_t>
constexpr type_t _max(type_t a, type_t b) {
    return (a>b) ? a : b;
}

template <typename type_t>
constexpr type_t _clamp(type_t lo, type_t in, type_t hi) {
    return (in<lo) ? lo :
                     ((in>hi) ? hi :
                                in);
}

/* Return clipped input between [-1,+1] */
constexpr float _clip(float in) {
    return _clamp(-1.f, in, 1.f);
}

/* Linear interpolation */
constexpr float _lerp(float a, float b, float i) {
    return a+(b-a) * i;
}

/* Fractional part */
constexpr float _fpart(float in) {
    return in-float(int32_t(in));
}

/* 64 bit xor shift pseudo random number generator */
uint64_t _rand64(uint64_t & x) {
    x ^= x>>12;
    x ^= x<<25;
    x ^= x>>27;
    return x;
}

/* Triangular noise distribution [-1,+1] tending to 0 */
float _dither(uint64_t & x) {
    static const uint32_t fmask = (1<<23)-1;
    union { float f; uint32_t i; } u, v;
    u.i = (uint32_t(_rand64(x)) & fmask)|0x3f800000;
    v.i = (uint32_t(_rand64(x)) & fmask)|0x3f800000;
    float out = (u.f+v.f-3.f);
    return out;
}
} // namespace {}

size_t sound_t::render(
    int32_t * l,
    int32_t * r,
    size_t length)
{
    // number of samples to output
    const size_t count = min2(data_.size()/2, length);
    // makeup gain from float to int16_t
    static const float c_gain = float(0x7000);
    // local copy of dither seed
    uint64_t dither = dither_;
    decimate_9_t & dec = decimate_;
    float dc = dc_;
    // mix down buffer to output region
    float * src = &data_[0];
    for (size_t i=0; i<count; ++i) {
        // decimate
        const float s0 = dec(src[0], src[1]);
        src += 2;
        // remove dc
        const float s1 = s0 - dc;
        dc = _lerp(dc, s1, 0.0001f);
        // clip
        const float s2 = _clip(s1);
        // apply makeup gain
        const float s3 = s2 * c_gain;
        // dither
        const float s4 = s3 + _dither(dither);
        // write to output buffer
        l[i] = int16_t(s4);
        r[i] = int16_t(s4);
    }
    // save dither seed back out
    dither_ = dither;
    dc_ = dc;
    // return number of samples written
    return count;
}

void audio_source_chip_t::render(const mix_out_t & mix) {
    // loop until all samples are emitted
    size_t remaining = mix.count_;
    while (remaining) {
        // wipe the buffer
        buffer_.clear();
        // find samples remaining
        size_t count = _min(remaining * 2, buffer_.size());

        // <-- todo factor in midi events

        // render from all audio sources
        {
            for (auto *src: source_blit_) {
                src->render(count, buffer_);
            }
            for (auto *src: source_lfsr_) {
                src->render(count, buffer_);
            }
            for (auto *src: source_pulse_) {
                src->render(count, buffer_);
            }
            for (auto *src: source_nestri_) {
                src->render(count, buffer_);
            }
        }
        // render out the samples in our buffer
        remaining -= buffer_.render(mix.left_, mix.right_, count / 2);
    }
}

size_t pulse_t::render(
    size_t length,
    sound_t &out)
{
    const float period = period_;
    const float offset = offset_;
    const float volume = volume_;
    const size_t count = _min(length, out.data().size());
    float accum = accum_;
    float * dst = out.data().data();
    // do not render if over nyquist
    if (period<=2.f) {
        return 0;
    }
    // while there are samples to render
    for (size_t i=0; i<count; ++i) {
        // tick the square wave counter
        if ((accum -= 1.f)<0.f) {
            accum += period;
        }
        // apply duty offset
        float a = accum-offset;
        // turn into pulse wave
        float b = a > 0.f ? 1.f : -1.f;
        // apply volume attenuation
        float c = b * volume;
        // move into output buffer
        dst[i] += c;
    }
    // copy period back to structure
    accum_ = accum;
    return count;
}

size_t nestri_t::render(size_t length, sound_t &out) {
#define A(X) ((X)/7.5f-1.f)
    static const std::array<float, 32> tri_table = {
        A(15),  A(14),  A(13),  A(12),  A(11), A(10), A(9),  A(8),
        A(7),   A(6),   A(5),   A(4),   A(3),  A(2),  A(1),  A(0),
        A(0),   A(1),   A(2),   A(3),   A(4),  A(5),  A(6),  A(7),
        A(8),   A(9),   A(10),  A(11),  A(12), A(13), A(14), A(15),
    };
#undef A
    float accum = accum_;
    const float delta = delta_;
    const float volume = volume_;
    const size_t count = _min(length, out.data().size());
    float * dst = out.data().data();
    // while there are samples to render
    for (size_t i=0; i<count; ++i) {
        // tick the counter
        if ((accum -= delta)<0.f) {
            accum += 32.f;
        }
#if 0
        size_t a = (size_t(accum)+0)&0x1f;
        size_t b = (size_t(accum)+1)&0x1f;
        float  i = accum-float(int(accum));
        float  v = _lerp(tri_table[a], tri_table[b], i);
#else
        // index into lookup table
        float  v = tri_table[size_t(accum)&0x1f];
#endif
        // apply volume attenuation
        float c = v * volume;
        // move into output buffer
        dst[i] += c;
    }
    // copy period back to structure
    accum_ = accum;
    return count;
}

size_t lfsr_t::render(size_t length, sound_t &out) {
    const float volume = volume_;
    // zero volume skip
    if (volume<=0.f)
        return 0;
    const uint32_t period = period_;
    const size_t count = _min(length, out.data().size());
    uint32_t reg = lfsr_;
    uint32_t counter = counter_;
    float * dst = out.data().data();
    // while there are samples to render
    for (size_t i=0; i<count; ++i) {
        // get current noise state
        float value = (reg&1) ? 1.f : -1.f;
        // apply attenuation
        dst[i] += value * volume;
        // if the shift register should be clocked
        if (counter==0) {
            // calculate new bit (taps{6,0})
            uint32_t bit = ((reg>>1)^reg);
            // shift out
            reg >>= 1;
            // shift in new bit
            reg |= (bit<<14)&0x4000;
            // reset counter
            counter += period;
        }
        else {
            // decrement the lfsr clock counter
            --counter;
        }
    }
    // copy shift register back into structure
    lfsr_ = reg;
    counter_ = counter;
    return count;
}

size_t blit_t::render(size_t samples, sound_t &buffer) {
    // defined in blip_table.cpp
    extern const float * g_blip_table;

    const uint32_t c_ring_size  = c_ring_size;
    const float    c_leak       = 0.999f;
    const uint32_t c_blip_count = 16;
    const uint32_t c_blip_size  = 32;

    assert(c_blip_size==c_ring_size);

    auto & ring = ring_;
    float accum = accum_;
    float out = out_;
    uint32_t index = index_;
    uint32_t edge = edge_;

    const float volume = volume_;

    if ((hcycle_[0]<=0.f)||(hcycle_[1]<=0.f)) {
        return 0;
    }

    assert(hcycle_[0] > 0.f);
    assert(hcycle_[1] > 0.f);

    const int32_t length = int32_t(_min(samples, buffer.data().size()));
    float * dst = buffer.data().data();
    // while there are samples left to render
    for (int32_t i=length; i>0;) {
        // do we need to output a blip
        while (accum < 0.f) {
            // scale factor for this edge and volume
            const float scale = (edge&1) ? volume : -volume;
            // flip pulse edge
            edge ^= 0x1;
            // find lerp data
            float    r = (1.f+accum) * float(c_blip_count);
            uint32_t a = int32_t(r+0) & (c_blip_count-1);
            uint32_t b = int32_t(r+1) & (c_blip_count-1);
            float    l = _fpart(r);
            // locate the blip tables that bracket this index
            const float * blip_a = &g_blip_table[a * c_blip_size];
            const float * blip_b = &g_blip_table[b * c_blip_size];
            // for all samples in the blip
            for (uint32_t i = 0; i<c_ring_size; ++i) {
                // lerp blip value
                float v = _lerp(blip_a[i], blip_b[i], l);
                // sum into blip ring buffer
                ring[(index+i)%c_ring_size] += v * scale;
            }
            // reset the period with this duty cycle period
            accum += hcycle_[edge&1];
        }
        // number of samples before next blip or end
        uint32_t interval = uint32_t(accum+1.f);
        uint32_t count = _min<uint32_t>(i, interval);
        assert(count > 0);
        i -= count;
        // advance the period by the amount we will render
        accum -= float(count);
        // render using the blip ring buffer
        for (uint32_t i = 0; i<count; ++i, ++dst) {
            // get this ring buffer slot
            float & slot = ring[(index++)%c_ring_size];
            // integrate using blip ring buffer
            out  += slot;
            out  *= c_leak;
            *dst += out;
            // clear ring buffer slot
            slot  = 0.f;
        }
    }
    // pack blip state back into struct
    out_   = out;
    accum_ = accum;
    index_ = index;
    edge_  = edge;
    return length;
}
