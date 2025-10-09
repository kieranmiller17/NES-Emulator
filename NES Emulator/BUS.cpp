#include "BUS.hpp"
#include "typedef.hpp"
#include "PPU.hpp"

u8 BUS::read(u16 addr) 
{
	if (addr <= 0x1FFF) {
		return ram[addr % 2048];
	}
	else if ((addr >= 0x2000) && (addr <= 0x3FFF)) {
 		return ppu.read(addr);}
	else if ((0x4020 <= addr) && (addr <= 0xFFFF)) {
		return rom.read(addr);
	}
}

void BUS::write(u16 addr, u8 data) 
{
	if (addr <= 0x1FFF) {
		ram[addr % 2048] = data;
	}
	else if ((addr >= 0x2000) && (addr <= 0x3FFF)) {
		return ppu.write(addr, data);}
	else if (addr == 0x4014) {
		dma_process_on = true;
	}
	else if ((0x4020 <= addr) && (addr <= 0xFFFF)) {
		rom.write(addr, data);
	}
}

