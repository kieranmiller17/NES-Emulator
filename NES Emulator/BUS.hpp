#ifndef BUS_H
#define BUS_H


#include <array>
#include "ROM.hpp"
#include "typedef.hpp"
#include "PPU.hpp"

class BUS
{
public:
	std::array<u8, 2048> ram {};
	ROM rom {};
	PPU ppu {};
	bool dma_process_on { false };

	u8 read(u16 addr);
	void write(u16 addr, u8 data);
};

#endif