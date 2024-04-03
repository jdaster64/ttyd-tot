#pragma once

#include <cstdint>

namespace gc {

struct vec3
{
	float x, y, z;
} ;

struct mtx34
{
    float m[3][4];
} ;

struct mtx44
{
    float m[4][4];
} ;

struct color4
{
	uint8_t r, g, b, a;
} ;

}