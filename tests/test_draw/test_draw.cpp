#include <SDL/SDL.h>

#include "../../framework_core/random.h"
#include "../../framework_draw/draw.h"

namespace {
    SDL_Surface * screen_;
    draw_t draw_;
    bitmap_t bitmap_;
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
    return true;
}

void test_circles() {
    draw_.colour_ = 0x202020;
    draw_.clear();
    draw_.viewport(recti_t {32, 32, 320-32, 240-32});
    for (int i=0; i<100; ++i) {
        const vec2i_t p = vec2i_t {
            int32_t(random_.randllu() % 320),
            int32_t(random_.randllu() % 240)};
        draw_.colour_ = random_.randllu();
        draw_.circle(p, random_.randllu() % 64);
    }
}

void test_lines() {
    draw_.colour_ = 0x202020;
    draw_.clear();
    draw_.viewport(recti_t {32, 32, 320-32, 240-32});
    for (int i=0; i<100; ++i) {
        const vec2f_t p0 = vec2f_t {
                float(random_.randllu() % 320),
                float(random_.randllu() % 240)};
        const vec2f_t p1 = vec2f_t {
                float(random_.randllu() % 320),
                float(random_.randllu() % 240)};
        draw_.colour_ = random_.randllu();
        draw_.line(p0, p1);
    }
}

void test_rect() {
    draw_.colour_ = 0x202020;
    draw_.clear();
    draw_.viewport(recti_t {32, 32, 320-32, 240-32});
    for (int i=0; i<100; ++i) {
        const vec2i_t p0 = vec2i_t {
            int32_t(random_.randllu() % 320),
            int32_t(random_.randllu() % 240)};
        const vec2i_t p1 = vec2i_t {
            int32_t(random_.randllu() % 320),
            int32_t(random_.randllu() % 240)};
        draw_.colour_ = random_.randllu();
        draw_.rect(recti_t{p0.x, p0.y, p1.x, p1.y});
    }
}

void test_plot() {
    draw_.colour_ = 0x202020;
    draw_.clear();
    draw_.viewport(recti_t {32, 32, 320-32, 240-32});
    for (int i=0; i<1000; ++i) {
        const vec2i_t p0 = vec2i_t {
                int32_t(random_.randllu() % 320),
                int32_t(random_.randllu() % 240)};
        draw_.colour_ = random_.randllu();
        draw_.plot(p0);
    }
}

void test_blit() {
    draw_.colour_ = 0x202020;
    draw_.clear();
    draw_.viewport(recti_t {32, 32, 320-32, 240-32});
    if (!sprite_.valid()) {
        if (!bitmap_t::load("/home/flipper/repos/ldjam/tests/test_draw/sprite1.bmp", sprite_)) {
            return;
        }
    }
    for (int i=0; i<100; ++i) {
        blit_info_t info;
        info.bitmap_ = &sprite_;
        info.dst_pos_ = vec2i_t {
            int32_t(random_.randllu() % 200),
            int32_t(random_.randllu() % 180)};
        info.src_rect_ = recti_t {0, 0, 31, 31};
        info.h_flip_ = false;
        info.type_ = e_blit_opaque;
        draw_.blit(info);
    }
}

struct test_t {
    const char * name_;
    void (*func_)();
};

#define STRINGY(X) #X
#define TEST(X) {STRINGY(X), X}

std::array<test_t, 5> tests = {{
    TEST(test_blit),
    TEST(test_circles),
    TEST(test_lines),
    TEST(test_plot),
    TEST(test_rect)
}};

int main(const int argc, char *args[]) {
    if (!init()) {
        return 1;
    }
    uint32_t test_index = 0;
    bool pause = false;
    bool active = true;
    while (active) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_LEFT) {
                    --test_index;
                }
                if (event.key.keysym.sym == SDLK_RIGHT) {
                    ++test_index;
                }
                if (event.key.keysym.sym == SDLK_SPACE) {
                    pause ^= true;
                }
            }
            if (event.type == SDL_QUIT) {
                active = false;
            }
        }
        if (!pause) {
            const auto test = tests[test_index % tests.size()];
            test.func_();
            draw_.render_2x(screen_->pixels, screen_->pitch / 4);
            SDL_Flip(screen_);
        }
        SDL_Delay(1000/25);
    }
    return 0;
}
