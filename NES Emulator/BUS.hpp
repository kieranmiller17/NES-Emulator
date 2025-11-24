#ifndef BUS_H
#define BUS_H

#include "typedef.hpp"
#include <array>

#include "ROM.hpp"
#include "PPU.hpp"
#include "controller.hpp"
#include "Mapper.hpp"
#include "NROM.hpp"

class BUS
{
public:

	~BUS() {
		delete rom;
	}

	u8 read(u16 addr);
	void write(u16 addr, u8 data);

	Mapper* rom {};
	PPU ppu {};
	Controller controller {};

private:

	std::array<u8, 2048> ram {};
	bool dma_process_on { false };
};

#endif