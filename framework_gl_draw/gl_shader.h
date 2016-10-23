#pragma once
#include <cstdint>
#include <map>
#include "../framework_core/string.h"
#include "../framework_core/vec2.h"
#include "../framework_core/vec3.h"
#include "../framework_core/mat2.h"
#include "gl_mat4.h"

namespace tengu {

struct gl_texture_t;
struct buffer_t;


struct gl_shader_t {

    gl_shader_t();
    ~gl_shader_t();

    bool load(const buffer_t &vert,
              const buffer_t &frag);

    bool bind(const char *name, const gl_texture_t &val, const uint32_t slot);
    bool bind(const char *name, const mat4f_t &val);
    bool bind(const char *name, const mat2f_t &val);
    bool bind(const char *name, const vec3f_t &val);
    bool bind(const char *name, const vec2f_t &val);
    bool bind(const char *name, float &val);

    bool get_attrib_loc(const string_t & name, int32_t & out) const {
        auto itt = attribs_.find(name);
        if (itt!=attribs_.end()) {
            out = itt->second;
        }
        return itt!=attribs_.end();
    }

    void release();

    operator bool() const;

protected:
    friend struct gl_draw_t;

    void _inspect_shader();

    std::map<string_t, int32_t, string_t::compare_t> uniforms_;
    std::map<string_t, int32_t, string_t::compare_t> attribs_;

    uint32_t vert_, frag_;
    uint32_t program_;
};
} // namespace tengu