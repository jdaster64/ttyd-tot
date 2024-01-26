#include "common_ui.h"

#include <ttyd/dispdrv.h>
#include <ttyd/fontmgr.h>
#include <ttyd/windowdrv.h>

#include <cstdint>
#include <cstring>

namespace mod {
    
namespace {

using ::ttyd::dispdrv::CameraId;

}

void RegisterDrawCallback(void (*func)(), CameraId camera_layer, float order) {
    ttyd::dispdrv::dispEntry(
        camera_layer, /* render_mode = */ 2, order, 
        [](CameraId camera_layer, void* user) {
            reinterpret_cast<void(*)()>(user)();
        }, reinterpret_cast<void*>(func));
}
    
void GetTextDimensions(
    const char* text, float scale, float* out_width, float* out_height) {
    uint16_t num_lines;
    uint16_t length = ttyd::fontmgr::FontGetMessageWidthLine(text, &num_lines);
    ++num_lines;
    
    *out_width = (length + 0) * scale;
    *out_height = (25 * num_lines - 9) * scale;
}
    
void DrawText(
    const char* text, float x, float y, uint8_t alpha, bool edge, 
    uint32_t color, float scale, int32_t alignment) {
    ttyd::fontmgr::FontDrawStart_alpha(alpha);
    ttyd::fontmgr::FontDrawColor(reinterpret_cast<uint8_t *>(&color));
    if (edge) {
        ttyd::fontmgr::FontDrawEdge();
    } else {
        ttyd::fontmgr::FontDrawEdgeOff();
    }
    ttyd::fontmgr::FontDrawScale(scale);
    
    if (alignment < 0 || alignment >= 9) alignment = 0;
    
    float width, height;
    GetTextDimensions(text, scale, &width, &height);
    // Set initial height of text based on top, middle, or bottom v-alignment.
    y += height * (alignment / 3) / 2;
    // Nudge up slightly so capital letters look ~exactly centered.
    y += 4.f * scale;
    
    char buf[100];
    int32_t line_start = 0;
    float line_x = 0;
    (void)buf;
    
    // Print the text a line at a time, if applicable.
    for (int32_t i = 0; text[i]; ++i) {
        if (text[i] == '\n') {
            // Copy this line to the temporary buffer and append a null byte.
            int32_t line_length = i - line_start < 99 ? i - line_start : 99;
            strncpy(buf, text + line_start, line_length);
            buf[line_length] = '\0';
            
            // Set individual lines' x-position based on h-alignment.
            GetTextDimensions(buf, scale, &width, &height);
            line_x = x - width * (alignment % 3) / 2;
            ttyd::fontmgr::FontDrawString(line_x, y, buf);
            
            // Advance to the next line.
            line_start = i + 1;
            y -= 25.f * scale;
        }
    }
    // Print the remainder of the string.
    GetTextDimensions(text + line_start, scale, &width, &height);
    line_x = x - width * (alignment % 3) / 2;
    ttyd::fontmgr::FontDrawString(line_x, y, text + line_start);
}
    
void DrawWindow(
    uint32_t color, float x, float y, float width, float height, 
    float corner_radius) {
    uint8_t *color_u8ptr   = reinterpret_cast<uint8_t *>(&color);
    ttyd::windowdrv::windowDispGX_Waku_col(
        0, color_u8ptr, x, y, width, height, corner_radius);
}
    
void DrawCenteredTextWindow(
    const char* text, float x, float y, uint8_t text_alpha, bool text_edge,
    uint32_t text_color, float text_scale, uint32_t window_color, 
    float window_pad, float window_corner_radius) {
    float width, height;
    GetTextDimensions(text, text_scale, &width, &height);
    
    DrawWindow(window_color,
               x - width / 2 - window_pad, y + height / 2 + window_pad,
               width + 2 * window_pad, height + 2 * window_pad,
               window_corner_radius);
    DrawText(text, x, y, text_alpha, text_edge, text_color, text_scale, 4);
}

}