#include "framework_gui.h"
#include "framework_gui_widgets.h"

enum gui_rgb_t {
  GUI_RGB_BG_1       = 0x1B2123,
  GUI_RGB_BG_2       = 0x232629,
  GUI_RGB_BG_3       = 0x31363B,
  GUI_RGB_BG_4       = 0x3F3F3F,
  GUI_RGB_PROGRESS   = 0x2180A9,
  GUI_RGB_SLIDER     = 0x605F5F,
  GUI_RGB_BORDER     = 0x76797C,
  GUI_RGB_TEXT       = 0xECECEC,
  GUI_RGB_SHADOW     = 0x101010
};

namespace tengu {
void gui_widget_frame_t::tick(gui_t &gui, 
                              gui_extern_io_t &io,
                              gui_extern_render_t &draw,
                              vec2i_t &origin) {
  draw.origin = origin;
  draw.rgb = GUI_RGB_BG_3;
  draw.draw_rect_fill(x0+1, y0+1, x1-1, y1-1);
  draw.rgb = GUI_RGB_BORDER;
  draw.draw_rect_outline(x0, y0, x1, y1);
  draw.rgb = GUI_RGB_SHADOW;
  draw.draw_shadow(x0, y0, x1, y1);
}

void gui_widget_button_t::tick(gui_t &gui,
                               gui_extern_io_t &io,
                               gui_extern_render_t &draw,
                               vec2i_t &origin) {
    // if this has focus
    if (gui.has_focus(this)) {
        gui_extern_io_t::mouse_state_t mouse;
        io.mouse_get(mouse);
        if (mouse.button[0] == gui_extern_io_t::IO_MOUSE_RELEASE) {
            // button was clicked
            gui_event_t e;
            e.type = GUI_EVENT_BUTTON_PUSHED;
            event.push_back(e);
        }
    }
    // redraw
    {
        draw.origin = origin;
        draw.rgb = GUI_RGB_BORDER;
        draw.draw_rect_outline(x0, y0, x1, y1);
    }
}

void gui_widget_check_box_t::tick(gui_t &gui,
                                  gui_extern_io_t &io,
                                  gui_extern_render_t &draw,
                                  vec2i_t &origin) {

    // if this has focus
    if (gui.has_focus(this)) {
      gui_extern_io_t::mouse_state_t mouse;
      io.mouse_get(mouse);
      if (mouse.button[0] == gui_extern_io_t::IO_MOUSE_RELEASE) {
        checked = !checked;
        // button was clicked
        gui_event_t e;
        e.type = checked ? GUI_EVENT_CHECKED : GUI_EVENT_UNCHECKED;
        event.push_back(e);
      }
    }
    // redraw
    {
        draw.origin = origin;
        draw.rgb = GUI_RGB_BG_2;
        draw.draw_rect_fill(x0 + 1, y0 + 1, x0 + width - 1, y0 + height - 1);
        draw.rgb = GUI_RGB_BORDER;
        draw.draw_rect_outline(x0, y0, x0 + width, y0 + height);
        if (checked) {
            draw.rgb = GUI_RGB_PROGRESS;
            draw.draw_circle(x0 + width / 2, y0 + height / 2, 3);
        }
    }
}

void gui_widget_hslider_t::tick(gui_t &gui,
                                gui_extern_io_t &io,
                                gui_extern_render_t &draw,
                                vec2i_t &origin) {

    const int32_t rad = size / 2;
    // if this has focus
    if (gui.has_focus(this)) {
        gui_extern_io_t::mouse_state_t mouse;
        io.mouse_get(mouse);
        if (mouse.button[0] == gui_extern_io_t::IO_MOUSE_DOWN) {
            // slide!
            const int32_t xlo = (origin.x + x0) + rad;
            const int32_t xhi = (origin.x + x1) - rad;
            const int32_t val = ((mouse.x - xlo) * max_value) / (xhi - xlo);
            value = clampv(0, val, max_value);
            // send update message
            gui_event_t e;
            e.type = GUI_EVENT_SLIDER;
            event.push_back(e);
        }
    }
    // redraw
    {
        draw.origin = origin;
        draw.rgb = GUI_RGB_BG_2;
        draw.draw_circle(x0 + rad, y0 + rad, rad);
        draw.draw_circle(x1 - rad, y0 + rad, rad);
        draw.draw_rect_fill(x0 + rad, y0, x1 - rad, y0 + size);
        const int32_t val = clampv(0, value, max_value);
        const int32_t max = maxv(1, max_value);
        const int32_t use_size = (x1 - x0) - rad * 2;
        const int32_t xv = (use_size * val) / max;
        draw.rgb = GUI_RGB_SLIDER;
        draw.draw_circle(x0 + rad + xv, y0 + rad, 3);
    }
}

void gui_widget_vslider_t::tick(gui_t &gui,
                                gui_extern_io_t &io,
                                gui_extern_render_t &draw,
                                vec2i_t &origin) {

    const int32_t rad = size / 2;
    // if this has focus
    if (gui.has_focus(this)) {
        gui_extern_io_t::mouse_state_t mouse;
        io.mouse_get(mouse);
        if (mouse.button[0] == gui_extern_io_t::IO_MOUSE_DOWN) {
            // slide!
            const int32_t ylo = (origin.y + y0) + rad;
            const int32_t yhi = (origin.y + y1) - rad;
            const int32_t val = ((mouse.y - ylo) * max_value) / (yhi - ylo);
            value = clampv(0, val, max_value);
            // send update message
            gui_event_t e;
            e.type = GUI_EVENT_SLIDER;
            event.push_back(e);
        }
    }
    // redraw
    {
        draw.origin = origin;
        draw.rgb = GUI_RGB_BG_2;
        draw.draw_circle(x0 + rad, y0 + rad, rad);
        draw.draw_circle(x0 + rad, y1 - rad, rad);
        draw.draw_rect_fill(x0, y0 + rad, x0 + size, y1 - rad);
        const int32_t val = clampv(0, value, max_value);
        const int32_t max = maxv(1, max_value);
        const int32_t use_size = (y1 - y0) - rad * 2;
        const int32_t yv = (use_size * val) / max;
        draw.rgb = GUI_RGB_SLIDER;
        draw.draw_circle(x0 + rad, y0 + rad + yv, 3);
    }
}
}
