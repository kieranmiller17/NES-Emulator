
#include "typedef.hpp"
#include "MMC3.hpp"

void MMC3::write(u16 addr, u8 data) {


	if (addr < 0x6000) {
		return;
	}
	if (addr < 0x8000) {
		wram[addr - 0x6000] = data;
		return;
	}

	addr &= 0xE001;


	if (addr == 0x8000) {
		bankSelect.val = data;
	}

	else if (addr == 0x8001) {

		switch (bankSelect.R) {

		case 0:
			CHRbank_2kb_1 = data & 0xFE;
			break;
		case 1:
			CHRbank_2kb_2 = data & 0XFE;
			break;
		case 2:
			CHRbank_1kb_1 = data;
			break;
		case 3:
			CHRbank_1kb_2 = data;
			break;
		case 4:
			CHRbank_1kb_3 = data;
			break;
		case 5:
			CHRbank_1kb_4 = data;
			break;
		case 6:
			PRGbank_1 = (data % prg_size);
			break;
		case 7:
			PRGbank_2 = (data % prg_size);
			break;
		default:
			break;
		}
	}

	else if (addr == 0xA000) {
		
		if ((data & 1) == 0) {
			screen_mirroring = Mirroring::Vertical;
		}

		else if ((data & 1) == 1) {
			screen_mirroring = Mirroring::Horizantal;
		}
	}

	else if (addr == 0xC000) {
		IRQ_reload_val = data;
	}

	else if (addr == 0xC001) {
		IRQ_reload = true;
		IRQ_counter = 0;
	}

	else if (addr == 0xE000) {
		IRQ_enabled = false;
	}

	else if (addr == 0xE001) {
		IRQ_enabled = true;
	}
}


u8 MMC3::read(u16 addr) {


	if (addr < 0x6000) {
		return 0;
	}
	else if (addr < 0x8000) {
		return wram[addr - 0x6000];
	}

	else if (addr < 0xA000) {

		if (bankSelect.P == 0) {
			u32 new_addr = (PRGbank_1 * 0x2000) + (addr - 0x8000);
			return prg_rom[new_addr];
		}
		else {
			u32 new_addr = prg_rom.size() - 0x4000 + (addr - 0x8000);
			return prg_rom[new_addr];
		}
	}

	else if (addr < 0xC000) {

		u32 new_addr = (PRGbank_2 * 0x2000) + (addr - 0xA000);
		return prg_rom[new_addr];
	}

	else if (addr < 0xE000) {

		if (bankSelect.P == 0) {
			u32 new_addr = prg_rom.size() - 0x4000 + (addr - 0xC000);
			return prg_rom[new_addr];
		}
		else {
			u32 new_addr = (PRGbank_1 * 0x2000) + (addr - 0xC000);
			return prg_rom[new_addr];
		}
	}

	else {
		u32 new_addr = prg_rom.size() - 0x2000 + (addr - 0xE000);
		return prg_rom[new_addr];
	}
}


u8 MMC3::CHR_read(u16 addr) {

	u32 new_addr {};

	if (bankSelect.C == 0) {

		if (addr < 0x800) {
			new_addr = (CHRbank_2kb_1 * 0x400) + addr;
		}
		else if (addr < 0x1000) {
			new_addr = (CHRbank_2kb_2 * 0x400) + (addr - 0x800);
		}
		else if (addr < 0x1400) {
			new_addr = (CHRbank_1kb_1 * 0x400) + (addr - 0x1000);
		}
		else if (addr < 0x1800) {
			new_addr = (CHRbank_1kb_2 * 0x400) + (addr - 0x1400);
		}
		else if (addr < 0x1C00) {
			new_addr = (CHRbank_1kb_3 * 0x400) + (addr - 0x1800);
		}
		else if (addr < 0x2000) {
			new_addr = (CHRbank_1kb_4 * 0x400) + (addr - 0x1C00);
		}
	}

	else {

		if (addr < 0x400) {
			new_addr = (CHRbank_1kb_1 * 0x400) + addr;
		}
		else if (addr < 0x800) {
			new_addr = (CHRbank_1kb_2 * 0x400) + (addr - 0x400);
		}
		else if (addr < 0xC00) {
			new_addr = (CHRbank_1kb_3 * 0x400) + (addr - 0x800);
		}
		else if (addr < 0x1000) {
			new_addr = (CHRbank_1kb_4 * 0x400) + (addr - 0xC00);
		}
		else if (addr < 0x1800) {
			new_addr = (CHRbank_2kb_1 * 0x400) + (addr - 0x1000);
		}
		else if (addr < 0x2000) {
			new_addr = (CHRbank_2kb_2 * 0x400) + (addr - 0x1800);
		}
	}

	u8 A12_curr_value = (addr >> 12) & 1;

	// Scanline counting
	if ((A12_curr_value == 1) && (A12_prev_value == 0)) {
		IRQ_counter -= 1;

		if (IRQ_reload) {
			IRQ_counter = IRQ_reload_val;
			IRQ_reload = false;
		}

		if (IRQ_counter == 0 && IRQ_enabled) {
			IRQ_request = true;
			IRQ_reload = true;
		}
	}

	A12_prev_value = A12_curr_value;
	

	return chr_rom[new_addr];

}