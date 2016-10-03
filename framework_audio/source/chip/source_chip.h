#pragma once
#include <stdint.h>
#include <array>
#include <queue>
#include <mutex>

#include "decimate.h"
#include "../../audio.h"

namespace source_chip {

struct env_ad_t {
    env_ad_t(float sample_rate_ = 44100.f)
        : ms_conv_(sample_rate_/1000.f)
        , attack_(1.f)
        , decay_(1.f)
        , env_(0.f)
        , one_shot_(false)
    {}

    void set_attack(float ms) {
        attack_ = 1.f/max_(1.f, ms * ms_conv_);
    }

    void set_decay(float ms) {
        decay_ = 1.f/max_(1.f, ms * ms_conv_);
    }

    void kill() {
        stage_ = e_end;
        env_ = 0.f;
    }

    void note_on(bool retrigger) {
        stage_ = e_attack;
        if (retrigger) {
            env_ = 0.f;
        }
    }

    void note_off() {
        if (stage_<e_decay) {
            stage_ = e_decay;
        }
    }

    float next() {
        switch (stage_) {
        case (e_attack):
            if ((env_ += attack_)>1.f) {
                env_ = 1.f;
                stage_ = one_shot_ ? e_decay : e_hold;
            }
            return env_;
        case (e_hold):
            return 1.f;
        case (e_decay):
            if ((env_ -= decay_)<0.f) {
                env_ = 0.f;
                stage_ = e_end;
            }
            return env_;
        case (e_end):
        default:
            return 0.f;
        }
    }

    float get_lin() const {
        return env_;
    }

    float get_exp() const {
        switch (stage_) {
        case (e_attack):
            return 1-(env_-1)*(env_-1);
        default:
            return 0+(env_-1)*(env_-1);
        }
    }

protected:
    enum {
        e_attack = 0,
        e_hold,
        e_decay,
        e_end,
    } stage_;
    float attack_, decay_;
    float env_;
    bool one_shot_;
    const float ms_conv_;

    static constexpr float max_(float a, float b) {
        return (a>b) ? a : b;
    }

    static constexpr float clamp_(float lo, float in, float hi) {
        return (in<lo) ? lo : ((in>hi) ? hi : in);
    }
};

struct lfo_sin_t {
    float a_;
    float s_[2];

    lfo_sin_t() {
        init(3.f, 44100.f);
    }

    void init(float freq, float sample_rate = 44100.f) {
        const bool stable = true;
        a_ = (stable) ? (2.f*C_PI*freq/sample_rate) :   // stable at very low freq.
                        (sinf(C_PI*freq/sample_rate));  // better for higher freq.
        s_[0] = 1.f;
        s_[1] = 0.f;
    }

    float tick() {
        s_[0] -= a_*s_[1];
        s_[1] += a_*s_[0];
        return s_[0];
    }

    float sin_part() const {
        return s_[0];
    }

    float cos_part() const {
        return s_[1];
    }

    void normalize() {
        const float mag = (s_[1]*s_[1]+s_[0]*s_[0]);
        const float nrm = (3.f-mag)*.5f; // approximation of 1/sqrt(x)
        s_[0] *= nrm;
        s_[1] *= nrm;
    }
};

struct sound_t {
    static const size_t _SIZE = 1024;

    sound_t()
        : decimate_()
        , dither_(1)
        , dc_(0.f) {
        data_.fill(0.f);
    }

    void clear() {
        data_.fill(0.f);
    }

    void reset() {
        data_.fill(0.f);
        dc_ = 0.f;
        dither_ = 1;
        decimate_.reset();
    }

    size_t size() const {
        return data_.size();
    }

    std::array<float, _SIZE> & data() {
        return data_;
    }

    const std::array<float, _SIZE> & data() const {
        return data_;
    }

    size_t render(int32_t * l,
                  int32_t * r,
                  size_t num_samples);

    size_t render(float * l,
                  size_t num_samples);

protected:
    decimate_9_t decimate_;
    uint64_t dither_;
    float dc_;
    std::array<float, _SIZE> data_;
};

struct event_t {
    enum : uint8_t {
        e_note_on,  // chan note vel
        e_note_off, // chan note
        e_cc,       // chan cc val
    };
    uint8_t type_;
    uint8_t data_[3];
};

typedef std::queue<event_t> event_queue_t;

struct pulse_t {

    pulse_t(event_queue_t & stream, float sample_rate = 44100.f * 2.f)
        : sample_rate_(sample_rate)
        , queue_(stream)
        , duty_(.5f)
        , volume_(.0f)
        , accum_(.0f)
        , period_(100.f)
        , vibrato_(0.f)
        , env_(sample_rate_)
    {
        set_freq(100.f, sample_rate_);
        lfo_.init(3.f, sample_rate_);
        env_.set_attack(1.f);
        env_.set_decay(1.f);
    }

    void set_duty(float duty) {
        duty_ = duty;
        offset_ = period_ * duty_;
    }

    void set_freq(float freq, float sample_rate) {
        freq_ = freq;
        period_ = sample_rate/freq;
        offset_ = period_ * duty_;
    }

    void render(size_t samples, sound_t & out);

    // <--- todo glide (legato)

protected:
    // internal render function
    size_t _render(size_t samples, sound_t & out);

    // cc values
    enum : uint8_t {
        e_attack = 0,
        e_decay,
        e_duty,
        e_vibrato
    };

    // event message handlers
    void on_note_on(const event_t & event);
    void on_note_off(const event_t & event);
    void on_cc(const event_t & event);

    const float sample_rate_;

    event_queue_t & queue_;
    env_ad_t env_;
    lfo_sin_t lfo_;
    float freq_, duty_, period_, offset_, accum_, volume_, vibrato_;
    // stereo
    float lvol_, rvol_;
};

#if 0
struct nestri_t {

    nestri_t(event_queue_t & stream)
        : queue_(stream)
        , volume_(0.f)
        , accum_(0.f)
    {
        set_freq(100.f, 44100.f);
    }

    void set_volume(float volume) {
        volume_ = volume;
    }

    void set_freq(float freq, float sample_rate) {
        delta_ = (32.f/sample_rate) * freq;
    }

    size_t render(size_t samples, sound_t & out);

protected:
    event_queue_t & queue_;
    float accum_;
    float delta_;
    float volume_;
};
#endif

struct lfsr_t {

    lfsr_t(event_queue_t & stream)
        : queue_(stream)
        , lfsr_(1)
        , period_(100)
        , counter_(100)
        , volume_(0.f) {
    }

    void set_volume(float volume) {
        volume_ = volume;
    }

    void set_period(uint32_t period) {
        period_ = period;
    }

    size_t render(size_t samples, sound_t & out);

protected:
    event_queue_t & queue_;
    env_ad_t env_;
    uint32_t lfsr_;
    uint32_t period_;
    uint32_t counter_;
    float volume_;
};

#if 0
struct blit_t {

    blit_t(std::queue<event_t> & stream)
        : queue_(stream)
        , duty_(.5f)
        , out_(0.f)
        , edge_(0)
        , index_(0)
        , volume_(0.f)
        , accum_(0.f)
        , period_(1.f)
    {
        for (uint32_t i = 0; i<ring_.size(); ++i) {
            ring_[i] = 0.f;
        }
        hcycle_[0] = 1.f;
        hcycle_[1] = 1.f;
    }

    void set_duty(float duty) {
        duty_ = duty;
        hcycle_[0] = period_ * duty_;
        hcycle_[1] = period_-hcycle_[0];
    }

    void set_freq(float freq, float sample_rate) {
        if (freq>0.f) {
            period_ = sample_rate/freq;
        }
        else {
            period_ = 1.f;
        }
        hcycle_[0] = period_ * duty_;
        hcycle_[1] = period_-hcycle_[0];
    }

    void set_volume(float volume) {
        volume_ = volume;
    }

    void render(size_t samples, sound_t & out);

protected:
    size_t _render(size_t samples, sound_t & out);
    
    event_queue_t & queue_;
    static const size_t c_ring_size = 32;
    float period_, duty_, accum_;
    float volume_, out_;
    int32_t edge_;
    float hcycle_[2];
    uint32_t index_;
    std::array<float, c_ring_size> ring_;
};
#endif

template <typename type_t>
struct queue_t {

    void push(const type_t & in) {
        std::lock_guard<std::mutex> guard(mutex_);
        q_.push(in);
    }

    bool pop(type_t & out) {
        std::lock_guard<std::mutex> guard(mutex_);
        if (q_.empty()) {
            return false;
        }
        else {
            out = q_.front();
            q_.pop();
            return true;
        }
    }

protected:
    std::queue<event_t> q_;
    std::mutex mutex_;
};

struct audio_source_chip_t:
    public audio_source_t {

    void init();

    virtual void render(const mix_out_t &) override;

    // render as mono float
    void render(float * out, size_t samples);

    // push an audio event
    void push(const event_t & event) {
        input_.push(event);
    }

protected:
    // thread safe input queue
    queue_t<event_t> input_;
    // mix buffer
    sound_t buffer_;
    // event buffer per channel
    std::array<std::queue<event_t>, 16> event_;
    // individual sound source
    std::vector<pulse_t*> source_pulse_;
};
} // namespace source_chip
