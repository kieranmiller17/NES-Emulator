
#include <stdexcept>
#include "PPU.hpp"
#include <time.h>
#include <SDL.h>


PPU::PPU(std::vector<u8> &chr, Mirroring m) {
	chr_rom = chr;
	mirroring = m;
	vram.resize(2048);
	OAMdata.resize(256);
	palette_table.resize(32);
	secondaryOAM.resize(32);
}


void PPU::tick() {

}

void PPU::render_visible_scanline() {

	if (dot == 0) { // Idle cycle - do nothing
	}

	else if (dot <= 256) {

		pushPixel();

		getPixelData();

		if ((dot % 8) == 0) {
			fill_shift_registers();
		}
	}

}

u8 PPU::get_background_pixel() {
	
	u8 p0 = ((patternLowRegister >> register_x) & 1);
	u8 p1 = ((patternHighRegister >> register_x) & 1);
	u8 p2 = ((attributeRegister1 >> register_x) & 1);
	u8 p3 = ((attributeRegister2 >> register_x) & 1);

	patternLowRegister >>= 1;
	patternHighRegister >>= 1;
	attributeRegister1 >>= 1;
	attributeRegister2 >>= 1;

	return (p3 << 3) | (p2 << 2) | (p1 << 1) | p0;

}


void PPU::pushPixel() {

	u8 bg_pixel = get_background_pixel();
	u8 pallette_index = read_PPU_data(bg_pixel);

}

void PPU::fill_shift_registers() {

	patternLowRegister |= (patternLowLatch << 8);
	patternHighRegister |= (patternHighLatch << 8);

	u8 quadrant = (((register_v.val >> 1) & 1) | ((register_v.val & 64) >> 5)) * 2;

	attributeRegister1 |= ((((attributeLatch >> quadrant) & 1) ? 0xFF : 1) << 8);
	attributeRegister2 |= ((((attributeLatch >> quadrant) & 2) ? 0xFF : 1) << 8);
}

void PPU::getPixelData() {
	if ((dot % 8) == 2) {
		nametableLatch = read_PPU_data(0x2000 | (register_v.val & 0x0FFF));
	}

	else if ((dot % 8) == 4) {
		attributeLatch = read_PPU_data(0x23C0 | (register_v.val & 0x0C00) | ((register_v.val >> 4) & 0x38) | ((register_v.val >> 2) & 0x07));
	}

	else if ((dot % 8) == 6) {
		u16 addr = (ctrl.B << 12) + (nametableLatch << 4) + (register_v.y);
		patternLowLatch = read_PPU_data(addr);
	}

	else if ((dot % 8) == 0) {
		u16 addr = (ctrl.B << 12) + (nametableLatch << 4) + 0b1000 + (register_v.y);
		patternHighLatch = read_PPU_data(addr);
	}
}
/*

void PPU::SpriteEvaluation() { // probably needs testing !!!

	if ((1 <= dot) && (dot <= 64)) {
		if ((dot % 2) == 0) {
			secondaryOAM[(dot/2) - 1] = 0xFF;
		}
	}
	else if ((65 <= dot) && (dot <= 256)) {

		if (OAMpointer >= 256) {
			return;
		}

		if (SecondaryOAMpointer < 32) { // If we have not yet found 8 sprites, fill the secondary OAM table
			if ((dot % 2) == 1) {
				OAMpointerData = OAMdata[OAMpointer];
			}
			else {
				secondaryOAM[SecondaryOAMpointer] = OAMpointerData;

				if (((OAMpointer % 4) == 0) && !((OAMpointerData <= scanline) && (OAMpointerData + 8 >= scanline))) { // If sprite not in range
					OAMpointer += 4;
				}
				else { // If sprite is in range
					OAMpointer += 1;
					SecondaryOAMpointer += 1;
				}
			}
		}

		else if (!spriteOverflow) { // If we have already found 8 sprites, continue to check for overflow, using bugged process
			if ((dot % 2) == 1) {
				OAMpointerData = OAMdata[OAMpointer];
			}

			else {
				if (((OAMpointer % 4) == 0) && !((OAMpointerData <= scanline) && (OAMpointerData + 8 >= scanline))) { // If sprite not in range
					OAMpointer += 4 + (((OAMpointer % 4) == 3) ? -3 : 1);
				}
				else { // Sprite is in range - so we have overflow
					spriteOverflow = true; // need to add register update here
				}
			}
		}
	}

	else if ((257 <= dot) && (dot <= 320)) {
		/// ????????????????
	}

}
 */

u8 PPU::read(u16 addr) {

	addr %= 8;

	if (addr == 2) {
		u8 copy { status.val };
		status.val &= 0x7F;
		register_w = 0;
		return copy;
	}
	else if (addr == 4) {
		return OAMdata[OAMaddr];
	}
	else if (addr == 7) {

		u8 result = read_buffer;
		
		if (register_v.val <= 0x2FFF) {
			read_buffer = read_PPU_data(register_v.val);
		}
		else {
			read_buffer = read_PPU_data(register_v.val - 0x1000);

			if (register_v.val >= 0x3F00) {
				return read_PPU_data(register_v.val);
			}
		}

		register_v.val += (ctrl.I ? 32 : 1);
		register_v.val %= 16384; 

		return result;
	}
}

void PPU::write(u16 addr, u8 data) {

	addr %= 8;

	if (addr == 0) {
		register_t.val &= 0x73FF;
		register_t.val |= ((data & 0b11) << 10);
		ctrl.val = data;
	}
	else if (addr == 1) {
		mask.val = data;
	}
	else if (addr == 3) {
		OAMaddr = data;
	}
	else if (addr == 4) {
		OAMdata[OAMaddr] = data;
		OAMaddr += 1;
	}
	else if (addr == 5) {
		if (register_w == 0) {
			register_t.val = (register_t.val & 0xFFE0) | (data >> 3);
			register_x = data & 0x07;
			register_w = 1;
		}
		else if (register_w == 1) {
			register_t.val = (register_t.val & 0x8C1F) | ((data & 0x07) << 12);
			register_t.val |= ((data >> 3) << 5);
			register_w = 0;
		}

		scroll = data;
	}
	else if (addr == 6) {
		if (register_w == 0) {
			register_t.val = (register_t.val & 0xFF) | ((data & 0x3F) << 8);
			register_w = 1;
		}
		else if (register_w == 1) {
			register_t.val |= data;
			register_v.val = register_t.val;
			register_w = 0;
		}
	}

	else if (addr == 7) {
		write_PPU_data(addr, data);

		register_v.val += (ctrl.I ? 32 : 1);
		register_v.val %= 16384;
	}
}


u8 PPU::read_PPU_data(u16 addr) {

	if (addr <= 0x1FFF) {
		return chr_rom[addr];
	}
	else if (addr <= 0x2FFF) {
		return vram[getMirrorAddress(addr)];
	}
	else if (addr <= 0x3EFF) {
		return vram[getMirrorAddress(addr - 0x1000)];
	}
	else if (addr <= 0x3FFF) {
		return palette_table[(addr - 0x3F00) % 32];
	}
	else { throw std::invalid_argument("not in range"); }

}

void PPU::write_PPU_data(u16 addr, u8 data) {

	if (addr <= 0x1FFF) {
		throw std::invalid_argument("can't write to CHR rom");
	}
	else if (addr <= 0x2FFF) {
		vram[getMirrorAddress(addr)] = data;
	}
	else if (addr <= 0x3EFF) {
		vram[getMirrorAddress(addr - 0x1000)] = data;
	}
	else if (addr <= 0x3FFF) {
		palette_table[(addr - 0x3F00) % 32] = data;
	}
	else { throw std::invalid_argument("not in range"); }

}

u16 PPU::getMirrorAddress(u16 addr) {

	if (mirroring == Mirroring::Horizantal) {
		if ((0x2000 <= addr) && (addr < 0x2800)) {
			addr %= 400;
		}
		else if ((0x2800 <= addr) && (addr < 0x3F00)) {
			addr %= 400;
			addr += 400;
		}
	}
	else if (mirroring == Mirroring::Vertical) {
		addr = (addr - 2000) % 800;
	}
	return addr;
}

void PPU::incrementCoarseX() { // from nesdev wiki 
	if (register_v.X == 31) {
		register_v.X = 0;
		register_v.N ^= 1;
	}
	else
		register_v.X++;
}

void PPU::incrementCoarseY() { // from nesdev wiki
	if ((register_v.val & 0x7000) != 0x7000) { 
		register_v.Y++;
	}
	else {
		register_v.val &= ~0x7000;                    
		if (register_v.Y == 29) {
			register_v.Y = 0;
			register_v.N ^= 2;
		}
		else if (register_v.Y == 31) {
			register_v.Y = 0;
		}
		else {
			register_v.Y++;
		}
	}
}