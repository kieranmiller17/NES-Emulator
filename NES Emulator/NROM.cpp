
#include "typedef.hpp"
#include "NROM.hpp"



u8 NROM::read(u16 addr)
{
	if (addr < 0x8000) {
		return 0;
	}
	else {
		addr -= 0x8000;
		if ((prg_rom.size() == 0x4000) and (addr >= 0x4000))
			addr %= 0x4000;

		return prg_rom[addr];
	};

};

void NROM::write(u16 addr, u8 data)
{
	return;
};