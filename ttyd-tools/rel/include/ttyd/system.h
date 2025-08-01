#pragma once

#include <cstdint>

namespace ttyd::system {

extern "C" {

void *memcpy_as4(void *dst, void *src, uint32_t size);
// mtxGetRotationElement

float sysMsec2FrameFloat(float milliseconds);
uint32_t sysMsec2Frame(uint32_t milliseconds);
float sysFrame2SecFloat(float frames);
// getV60FPS
void sysDummyDraw(float matrix[3][4]);
void sysWaitDrawSync();
uint16_t sysGetToken();

uint32_t irand(uint32_t range);
void movePos(float magnitude, float direction, float *x, float *y);
void sincosf(float angle, float *sin, float *cos);

uint8_t padGetRumbleStatus(uint32_t padId);
void padRumbleHardOff(uint32_t padId);
void padRumbleOff(uint32_t padId);
void padRumbleOn(uint32_t padId);

// Signed bytes, but actually returned as uints.
uint32_t keyGetSubStickY(uint32_t padId);
uint32_t keyGetStickY(uint32_t padId);
uint32_t keyGetStickX(uint32_t padId);

uint32_t keyGetButtonTrg(uint32_t padId);
uint32_t keyGetDirTrg(uint32_t padId);
uint32_t keyGetButtonRep(uint32_t padId);
uint32_t keyGetDirRep(uint32_t padId);
uint32_t keyGetButton(uint32_t padId);
uint32_t keyGetDir(uint32_t padId);
void makeKey();

void qqsort(void* data, uint32_t num_entries, uint32_t entry_size, void* comparator);
double intplGetValue(
    double start, double end, int32_t ease_mode, int32_t step, int32_t num_steps);
double angleABf(double x_start, double z_start, double x_end, double z_end);
// compAngle
double distABf(double x_start, double z_start, double x_end, double z_end);
// reviseAngle
const char *getMarioStDvdRoot();

}

}