
#include "typedef.hpp"
#include "UxROM.hpp"

u8 UxROM::read(u16 addr) {

	if (addr < 0x8000) {
		return 0;
	}

	u32 new_addr { 0 };

	if (addr < 0xC000) {
		new_addr = (bankSelect * 16384)  + (addr - 0x8000);
	}
	else {
		new_addr = (addr - 0xC000) + (banks - 1) * 16384;
	}

	return prg_rom[new_addr];

}



void UxROM::write(u16 addr, u8 data) {


	if (addr < 0x8000) {
		return;
	}

	if (banks == 16) {
		bankSelect = data & 15;
	}
	else {
		bankSelect = data & 7;
	}



}