#pragma once

#include <ttyd/dispdrv.h>

#include <cstdint>

namespace mod {

void RegisterDrawCallback(
    void (*func)(), ttyd::dispdrv::CameraId camera_layer, float order = 0.f);
    
void GetTextDimensions(
    const char* text, float scale, float* out_width, float* out_height);
    
void DrawText(
    const char* text, float x, float y, uint8_t alpha, bool edge,
    uint32_t color, float scale, int32_t alignment = 0);
    
void DrawWindow(
    uint32_t color, float x, float y, float width, float height, 
    float corner_radius);
    
void DrawCenteredTextWindow(
    const char* text, float x, float y, uint8_t text_alpha, bool text_edge,
    uint32_t text_color, float text_scale, uint32_t window_color, 
    float window_pad, float window_corner_radius);

}