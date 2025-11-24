
#include "typedef.hpp"
#include "MMC1.hpp"
#include "Mapper.hpp"

void MMC1::write(u16 addr, u8 data) {

	if (addr < 0x6000) {
		return;
	}
	if (addr < 0x8000) {
		if (!(prgBank & 0x10)) {
			wram[addr - 0x6000] = data;
		}
		return;
	}

	if (data & 128) {
		shiftRegister = 0x10;
		controlRegister.val |= 0xC;
		return;
	}

	if (shiftRegister & 1) {

		shiftRegister = (shiftRegister >> 1) | ((data & 1) << 4);

		if (addr <= 0x9FFF) {
			controlRegister.val = shiftRegister;

			switch (controlRegister.M) {
			case 0:
				screen_mirroring = Mirroring::SingleScreen_Lower;
				break;
			case 1:
				screen_mirroring = Mirroring::SingleScreen_Upper;
				break;
			case 2:
				screen_mirroring = Mirroring::Vertical;
				break;
			case 3:
				screen_mirroring = Mirroring::Horizantal;
				break;
			}

		}

		else if ((addr >= 0xA000) && (addr <= 0xBFFF)) {
			chrBank_0 = shiftRegister;
		}

		else if ((addr >= 0xC000) && (addr <= 0xDFFF)) {
			chrBank_1 = shiftRegister;
		}

		else if ((addr >= 0xE000) && (addr <= 0xFFFF)) {
			prgBank = shiftRegister;
		}

		shiftRegister = 0x10;
	}

	else {
		shiftRegister = (shiftRegister >> 1) | ((data & 1) << 4);
	}

}

u8 MMC1::read(u16 addr) {
	
	if (addr < 0x6000) {
		return 0;
	}

	if (addr < 0x8000) {
		if (!(prgBank & 0x10)) {
			return wram[addr - 0x6000];
		}
		return 0;
	}


	if (controlRegister.P <= 1) {
		u32 new_addr = (prgBank & 0xE) * 0x8000 + (addr - 0x8000);
		return prg_rom[new_addr];
	}
	else if (controlRegister.P == 2) {
		if (addr < 0xC000) {
			return prg_rom[addr - 0x8000];
		}
		else {
			u32 new_addr = (prgBank & 0xF) * 0x4000 + (addr - 0xC000);
			return prg_rom[new_addr];
		}
	}
	else if (controlRegister.P == 3) {
		if (addr < 0xC000) {
			u32 new_addr = (prgBank & 0xF) * 0x4000 + (addr - 0x8000);
			return prg_rom[new_addr];
		}
		else {
			u32 new_addr = prg_rom.size() - 0x4000 + (addr - 0xC000);
			return prg_rom[new_addr];
		}
	}

}

void MMC1::CHR_write(u16 address, u8 data) {
	chr_rom[address] = data;
}


u8 MMC1::CHR_read(u16 addr) {

	if (controlRegister.C == 0) {

		u32 new_addr = ((chrBank_0 & 0x1E) * 0x1000) + addr;
		return chr_rom[new_addr];
	}

	else if (controlRegister.C == 1) {

		if (addr < 0x1000) {
			u32 new_addr = (chrBank_0 * 0x1000) + addr;
			return chr_rom[new_addr];
		}
		else {
			u32 new_addr = (chrBank_1 * 0x1000) + (addr - 0x1000);
			return chr_rom[new_addr];
		}

	}

}