#include <array>
#define _SDL_main_h
#include <SDL/SDL.h>

#include "../../framework_core/timer.h"
#include "../../framework_core/random.h"
#include "../../framework_draw/draw.h"

namespace {

uint64_t get_time() {
    return SDL_GetTicks();
}

float scroll_ = 0.f;

timing_t<uint64_t> timer_ = timing_t<uint64_t>(get_time, 1000 / 30);
SDL_Surface * screen_;
draw_t draw_;
bitmap_t bitmap_;
bitmap_t font_;
random_t random_(0x12345);
bitmap_t sprite_;
}

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO)) {
        return false;
    }
    screen_ = SDL_SetVideoMode(320*2, 240*2, 32, 0);
    if (!screen_) {
        return false;
    }
    if (!bitmap_t::create(320, 240, bitmap_)) {
        return false;
    }
    if (!bitmap_.valid()) {
        return false;
    }
    draw_.set_target(bitmap_);
    timer_.reset();
    return true;
}

void test_circles() {
    draw_.viewport(recti_t {32, 32, 320-32, 240-32});
    for (int i=0; i<100; ++i) {
        const vec2i_t p = vec2i_t {
            int32_t(random_.rand<uint32_t>() % 320u),
            int32_t(random_.rand<uint32_t>() % 240u)};
        draw_.colour_ = random_.rand<int32_t>();
        draw_.circle(p, random_.rand<uint32_t>() % 64);
    }
}

void test_lines() {
    draw_.colour_ = 0x202020;
    draw_.clear();
    draw_.viewport(recti_t {32, 32, 320-32, 240-32});
    for (int i=0; i<100; ++i) {
        const vec2f_t p0 = vec2f_t {
            float(random_.rand<uint32_t>() % 320u),
            float(random_.rand<uint32_t>() % 240u)};
        const vec2f_t p1 = vec2f_t {
            float(random_.rand<uint32_t>() % 320u),
            float(random_.rand<uint32_t>() % 240u)};
        draw_.colour_ = random_.rand<int32_t>();
        draw_.line(p0, p1);
    }
}

void test_rect() {
    draw_.viewport(recti_t {32, 32, 320-32, 240-32});
    for (int i=0; i<100; ++i) {
        const vec2i_t p0 = vec2i_t {
            int32_t(random_.rand<uint32_t>() % 320u),
            int32_t(random_.rand<uint32_t>() % 240u)};
        const vec2i_t p1 = vec2i_t {
            int32_t(random_.rand<uint32_t>() % 320u),
            int32_t(random_.rand<uint32_t>() % 240u)};
        draw_.colour_ = random_.rand<int32_t>();
        draw_.rect(recti_t{p0.x, p0.y, p1.x, p1.y});
    }
}

void test_plot() {
    draw_.viewport(recti_t {32, 32, 320-32, 240-32});
    for (int i=0; i<1000; ++i) {
        const vec2i_t p0 = vec2i_t {
            int32_t(random_.rand<uint32_t>() % 320u),
            int32_t(random_.rand<uint32_t>() % 240u)};
        draw_.colour_ = random_.rand<int32_t>();
        draw_.plot(p0);
    }
}

void test_blit() {
    draw_.viewport(recti_t {32, 32, 320-32, 240-32});
    if (!sprite_.valid()) {
        if (!bitmap_t::load("assets/sprite1.bmp", sprite_)) {
            return;
        }
    }
    draw_.key_ = 0x0;
    for (int i=0; i<100; ++i) {
        blit_info_t info;
        info.bitmap_ = &sprite_;
        info.dst_pos_ = vec2i_t {
            int32_t(random_.rand<uint32_t>() % 320u),
            int32_t(random_.rand<uint32_t>() % 240u)};
        info.src_rect_ = recti_t {0, 0, 31, 31};
        info.h_flip_ = (random_.rand() & 1) == 0;
        info.type_ = e_blit_key;
        draw_.colour_ = random_.rand<uint32_t>();
        draw_.blit(info);
    }
}

void test_font() {
    draw_.viewport(recti_t {32, 32, 320-32, 240-32});
    if (!font_.valid()) {
        if (!bitmap_t::load("assets/font.bmp", font_)) {
            return;
        }
    }
    font_t font;
    font.bitmap_ = &font_;
    font.cellw_ = 9;
    font.cellh_ = 14;
    font.spacing_ = 9;
    draw_.key_ = 0xffffff;
    for (int i=0; i<100; ++i) {
        const vec2i_t p0 = vec2i_t {
                int32_t(random_.rand<uint32_t>() % 320u),
                int32_t(random_.rand<uint32_t>() % 240u)};
        draw_.colour_ = random_.rand<int32_t>();
        font.spacing_ = random_.rand_range(6, 32);
        draw_.printf(font, p0, "Hello World");
    }
}

void test_triangle() {
    draw_.viewport(recti_t {32, 32, 320-32, 240-32});
    for (int i=0; i<100; ++i) {
        const vec2f_t p0 = vec2f_t {
            random_.randfu() * 320.f,
            random_.randfu() * 240.f};
        const vec2f_t p1 = vec2f_t {
            random_.randfu() * 320.f,
            random_.randfu() * 240.f};
        const vec2f_t p2 = vec2f_t {
            random_.randfu() * 320.f,
            random_.randfu() * 240.f};
        draw_.colour_ = random_.rand<uint32_t>();
        draw_.triangle(p0, p1, p2);
    }
}

void test_tilemap() {
    if (!font_.valid()) {
        if (!bitmap_t::load("assets/font.bmp", font_)) {
            return;
        }
    }

    const float SPEED = 0.033f;
    scroll_ += scroll_ > C_2PI ? SPEED-C_2PI : SPEED;

    static const int _WIDTH = 34;
    static const int _HEIGHT = 16;

    int32_t ox = 16+cosf(scroll_)*16;
    int32_t oy = 16-sinf(scroll_)*16;

    draw_.viewport(recti_t {ox, oy, ox+320-32, oy+240-32});
    draw_.colour_ = 0x224477;
    draw_.rect(recti_t{0, 0, 320, 240});

    std::array<uint8_t, _WIDTH*_HEIGHT> tdata;
    for (int i = 0; i<tdata.size(); ++i) {
        auto & cell = tdata[i];
        cell = i % _WIDTH + (i / _WIDTH) + scroll_ * 10;
    }

    tilemap_t tiles = {
        vec2i_t { _WIDTH, _HEIGHT },
        vec2i_t { 9, 14 },
        tdata.data(),
        &font_,
        e_blit_opaque
    };

    vec2i_t pos{int32_t(sinf(scroll_) * 32.f),
                int32_t(cosf(scroll_) * 32.f)};
    draw_.blit(tiles, pos);
}

struct test_t {
    const char * name_;
    void (*func_)();
};

#define STRINGY(X) #X
#define TEST(X) {STRINGY(X), X}

std::array<test_t, 8> tests = {{
    TEST(test_font),
    TEST(test_blit),
    TEST(test_circles),
    TEST(test_lines),
    TEST(test_plot),
    TEST(test_rect),
    TEST(test_triangle),
    TEST(test_tilemap)
}};

int main(const int argc, char *args[]) {
    if (!init()) {
        return 1;
    }
    int32_t test_index = 1;
    bool pause = false;
    bool active = true;
    while (active) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_LEFT) {
                    if (--test_index<0) {
                        test_index += tests.size();
                    }
                }
                if (event.key.keysym.sym == SDLK_RIGHT) {
                    if ((++test_index) >= int32_t(tests.size())) {
                        test_index -= tests.size();
                    }
                }
                if (event.key.keysym.sym == SDLK_SPACE) {
                    pause ^= true;
                }
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    active = false;
                }
            }
            if (event.type == SDL_QUIT) {
                active = false;
            }
        }
        if (!pause) {
            if (timer_.frame()) {
                // clear the screen
                draw_.colour_ = 0x202020;
                draw_.clear();
                // execute the test case
                const auto test = tests[test_index % tests.size()];
                test.func_();
                //
                draw_.viewport(recti_t {0, 0, 320, 240});
                for (int i = 0; i<tests.size(); ++i) {
                    draw_.colour_ = i==test_index ? 0xffffff : 0x909090;
                    draw_.rect(recti_t(1 + 4 * i, 1, 3, 4, recti_t::e_relative));
                }
                // render to the screen
                draw_.render_2x(screen_->pixels, screen_->pitch/4);
                SDL_Flip(screen_);
            }
        }
        SDL_Delay(1);
    }
    return 0;
}
