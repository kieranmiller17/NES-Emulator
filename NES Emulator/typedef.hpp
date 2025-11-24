#ifndef TYPEDEF_H
#define TYPEDEF_H

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t s8;
typedef int16_t s16;

enum class Mirroring {
	Horizantal,
	Vertical,
	SingleScreen_Upper,
	SingleScreen_Lower,
};

#endif