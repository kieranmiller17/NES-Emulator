

#include "typedef.hpp"
#include <stdexcept>

#include <array>
#include <vector>
#include <string>
#include <iostream>

#include <SDL.h>

#include "PPU.hpp"
#include "Mapper.hpp"
#include "screen.hpp"


static std::vector<u8> SYSTEM_PALETTE {
	0x80, 0x80, 0x80, 0x00, 0x3D, 0xA6, 0x00, 0x12, 0xB0, 0x44, 0x00, 0x96, 0xA1, 0x00, 0x5E,
	0xC7, 0x00, 0x28, 0xBA, 0x06, 0x00, 0x8C, 0x17, 0x00, 0x5C, 0x2F, 0x00, 0x10, 0x45, 0x00,
	0x05, 0x4A, 0x00, 0x00, 0x47, 0x2E, 0x00, 0x41, 0x66, 0x00, 0x00, 0x00, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0xC7, 0xC7, 0xC7, 0x00, 0x77, 0xFF, 0x21, 0x55, 0xFF, 0x82, 0x37, 0xFA,
	0xEB, 0x2F, 0xB5, 0xFF, 0x29, 0x50, 0xFF, 0x22, 0x00, 0xD6, 0x32, 0x00, 0xC4, 0x62, 0x00,
	0x35, 0x80, 0x00, 0x05, 0x8F, 0x00, 0x00, 0x8A, 0x55, 0x00, 0x99, 0xCC, 0x21, 0x21, 0x21,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0xFF, 0xFF, 0xFF, 0x0F, 0xD7, 0xFF, 0x69, 0xA2, 0xFF,
	0xD4, 0x80, 0xFF, 0xFF, 0x45, 0xF3, 0xFF, 0x61, 0x8B, 0xFF, 0x88, 0x33, 0xFF, 0x9C, 0x12,
	0xFA, 0xBC, 0x20, 0x9F, 0xE3, 0x0E, 0x2B, 0xF0, 0x35, 0x0C, 0xF0, 0xA4, 0x05, 0xFB, 0xFF,
	0x5E, 0x5E, 0x5E, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0xFF, 0xFF, 0xFF, 0xA6, 0xFC, 0xFF,
	0xB3, 0xEC, 0xFF, 0xDA, 0xAB, 0xEB, 0xFF, 0xA8, 0xF9, 0xFF, 0xAB, 0xB3, 0xFF, 0xD2, 0xB0,
	0xFF, 0xEF, 0xA6, 0xFF, 0xF7, 0x9C, 0xD7, 0xE8, 0x95, 0xA6, 0xED, 0xAF, 0xA2, 0xF2, 0xDA,
	0x99, 0xFF, 0xFC, 0xDD, 0xDD, 0xDD, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 };

static unsigned char lookup[16] = {
0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf, };

PPU::PPU(Mapper* pointer, std::vector<u8> &chr) {
	rom = pointer;
	chr_rom = chr;
	vram.resize(2048);
	OAMdata.resize(256);
	palette_table.resize(32);
	secondaryOAM.resize(32);
	spriteRegisters.resize(32);
}


void PPU::tick() { // 261 scanlines, 341 dots per scanline

	render();

	cycles++;

	dot = (dot + 1) % 341;

	if (dot == 0) {
		scanline = (scanline + 1) % 262;
	}
}


void PPU::render() { 

	if (scanline <= 239) {
		visible_scanline();  // Select background pixel, choose between background and sprite pixel, then emit the pixel
		SpriteEvaluation();	 // Simultaneously select sprite pixel data for the NEXT scanline
	}
	else if (scanline == 241) {
		postrender_scanline();
	}
	else if (scanline == 261) {
		prerender_scanline();
	}
}


u8 PPU::read(u16 addr) {

	addr %= 8;

	if (addr == 2) {

		if (scanline == 241 && dot == 1) {  // If read at exact time as vblank, NMI is skipped
			skipVBlank = true;
		}

		u8 copy { status.val };
		status.V = 0;
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
				result = read_PPU_data(register_v.val);
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

		if (!ctrl.V && (data & 128 && status.V)) { immediate_NMI = true; }  // Trigger immediate NMI if Control Register Vblank NMI flag turmed on during VBlank - overriding any current NMI

		ctrl.val = data;

		register_t.N = ctrl.N;
	}
	else if (addr == 1) {
		mask.val = data;
	}
	else if (addr == 3) {
		OAMaddr = data;
	}
	else if (addr == 4) {
		OAMdata[OAMaddr++] = data;

	}
	else if (addr == 5) {

		if (register_w == 0) {
			register_t.X = ((data & 0xF8)>>3);
			register_x = (data & 0x7);
			register_w = 1;
		}
		else if (register_w == 1) {
			register_t.Y = ((data & 0xF8)>>3);
			register_t.y = (data & 0x07);
			register_w = 0;
		}

	}
	else if (addr == 6) {
		if (register_w == 0) {
			register_t.val &= 255;
			register_t.val |= ((data & 0x3F) << 8);
			register_w = 1;
		}
		else {
			register_t.val &= 0xFF00;
			register_t.val |= data;
			register_v.val = register_t.val;

			register_w = 0;
		}
	}

	else if (addr == 7) {
		write_PPU_data(register_v.val, data);

		register_v.val += (ctrl.I ? 32 : 1);
		register_v.val %= 16384;
	}
}


u8 PPU::read_PPU_data(u16 addr) {

	if (addr <= 0x1FFF) {
		rom->CHR_read(addr);
	}
	else if (addr <= 0x2FFF) {
		return vram[getMirrorAddress(addr)];
	}
	else if (addr <= 0x3EFF) {
		return vram[getMirrorAddress(addr - 0x1000)];
	}
	else if (addr <= 0x3FFF) {
		return read_palette_data(addr & 0x1f);
	}
	else { throw std::invalid_argument("not in range"); }

}

void PPU::write_PPU_data(u16 addr, u8 data) {

	if (addr <= 0x1FFF) { // Writable for custom roms using CHR_RAM 
		rom->CHR_write(addr, data);
	}
	else if (addr <= 0x2FFF) {
		vram[getMirrorAddress(addr)] = data;
	}
	else if (addr <= 0x3EFF) {
		vram[getMirrorAddress(addr - 0x1000)] = data;
	}
	else if (addr <= 0x3FFF) {
		write_palette_data(addr & 0x1F, data);
	}
	else { throw std::invalid_argument("not in range"); }

}

u8 PPU::read_palette_data(u16 addr) {
	switch (addr) {
	case 0x00:
	case 0x04:
	case 0x08:
	case 0x0C:
	case 0x10:
	case 0x14:
	case 0x18:
	case 0x1C:
		return palette_table[0];
		break;
	default:
		return palette_table[addr];
		break;
	}
}

void PPU::write_palette_data(u16 addr, u8 data) {

	data &= 0x3f;

	switch (addr) {
	case 0x00:
	case 0x04:
	case 0x08:
	case 0x0C:
	case 0x10:
	case 0x14:
	case 0x18:
	case 0x1C:
		palette_table[addr & 0xF] = data;
		break;
	default:
		palette_table[addr] = data;
		break;
	}
}


void PPU::writeDMA(u8 data) {
	OAMdata[OAMaddr++] = data;
}


void PPU::prerender_scanline() {

	if (dot == 0) {

		if ((mask.b || mask.s) && (odd_cycle)) { // Odd cycle - PPU skips tick every other cycle 
			dot++;
		}
		odd_cycle = !odd_cycle;
	}

	if (dot == 1) { // Reset for new frame
		sprite_zero_hit = false;
		status.val = 0;
		skipVBlank = false;
	}

	else if ((dot >= 8) && (dot <= 256)) {



		if (mask.b || mask.s) {

			if ((dot % 8) == 0) {
				incrementCoarseX();
			}
			if (dot == 256) {
				incrementCoarseY();
			}
		}
	}

	else if (dot == 257) {

		if (mask.b || mask.s) {
			register_v.X = register_t.X;
			register_v.N = (register_v.N & 0x2) | (register_t.N & 0x1);
		}
	}

	else if ((dot >= 280) && (dot <= 304)) {

		if (mask.b || mask.s) {
			register_v.Y = register_t.Y;
			register_v.y = register_t.y;
			register_v.N = (register_v.N & 0x1) | (register_t.N & 0x2);
		}
	}

	else if ((dot >= 321) && (dot <= 336)) {

		if (mask.b || mask.s) {
			getPixelData();
		}
	}
}


void PPU::visible_scanline() {

	if (dot == 0) { // Idle cycle - do nothing
	}

	else if (dot <= 256) {

		if (mask.b || mask.s) {


			u8 bg_pixel = get_background_pixel();
			u8 sprite_pixel = get_sprite_pixel(); 
			u8 pixel = choose_pixel(bg_pixel, sprite_pixel);
			pushPixel(pixel);
		
			getPixelData();  

		}
	}

	else if (dot <= 320) {

		if (mask.b || mask.s) {

			if (dot == 257) {  // Reload register_v with data saved to register_t for vertical blank
				register_v.X = register_t.X;  
				register_v.N = (register_v.N & 0x2) | (register_t.N & 0x1);
			}
		}
	}

	else if (dot <= 336) {

		if (mask.b || mask.s) {
			getPixelData(); // Pixel data for start of next scanline
		}
	}

}

void PPU::postrender_scanline() {

	if (dot == 0) {
		screen::updateScreen();
	}

	if ((dot == 1) && !skipVBlank) {
		status.V = 1;
	
		if (ctrl.V == 1) {
			NMI_enabled = true;
		}
	}

}

void PPU::getPixelData() {

	patternLowRegister >>= 1;
	patternHighRegister >>= 1;
	attributeRegister1 >>= 1;
	attributeRegister2 >>= 1;

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
		u16 addr = (ctrl.B << 12) + (nametableLatch << 4) + 8 + (register_v.y);
		patternHighLatch = read_PPU_data(addr);

		fill_shift_registers();
		incrementCoarseX();

		if (dot == 256) {
			incrementCoarseY();
		}
	}
}


u8 PPU::get_background_pixel() {
	
	u16 p0 = ((patternLowRegister >> register_x) & 1);
	u16 p1 = ((patternHighRegister >> register_x) & 1);
	u16 p2 = ((attributeRegister1 >> register_x) & 1);
	u16 p3 = ((attributeRegister2 >> register_x) & 1);

	return (p3 << 3) | (p2 << 2) | (p1 << 1) | p0;
}



u8 PPU::get_sprite_pixel() {

	for (u16 i { 0 }; i < 32; i += 4) {

		if ((spriteRegisters[i] <= dot - 1) && (dot <= spriteRegisters[i] + 8)) {

			sprite_priority = ((spriteRegisters[i+1] & 32) ? false : true);

			u8 x_step = (dot - 1 - spriteRegisters[i]);
			u8 reverse_horizantally = spriteRegisters[i + 1] & 64;

			u8 p0 = (spriteRegisters[i + 2] >> (reverse_horizantally ? x_step : (7 - x_step))) & 1;
			u8 p1 = (spriteRegisters[i + 3] >> (reverse_horizantally ? x_step : (7 - x_step))) & 1;

			if (!(p0 || p1)) {
				continue;
			}

			u8 p2 = (spriteRegisters[i + 1] & 3);

			return (p2 << 2) | (p1 << 1) | p0;
		}

	}

	return 0;

}



u8 PPU::choose_pixel(u8 bg_pixel, u8 sprite_pixel) {

	u8 pixel {};

	if (((bg_pixel & 3) != 0) && ((sprite_pixel & 3) != 0)) {  

		if (sprite_zero_hit && !(status.S || ((dot <= 8) && !(mask.m && mask.M)) || (dot == 256))) {
			status.S = 1;
		}
		pixel = (sprite_priority ? (0x10 | sprite_pixel) : bg_pixel);
	}

	else if ((((bg_pixel & 3) != 0) && ((sprite_pixel & 3) == 0)) || dot == 256) {
		pixel = bg_pixel;
	}

	else if (((bg_pixel & 3) == 0) && ((sprite_pixel & 3) != 0) && mask.s) {
		pixel = (0x10 | sprite_pixel);
	}

	else {
		pixel = 0;
	}

	return pixel;

}



void PPU::fill_shift_registers() {

	patternLowRegister |= (reverse(patternLowLatch) << 8);
	patternHighRegister |= (reverse(patternHighLatch) << 8);

	u8 quadrant = (((register_v.X >> 1) & 1) | (register_v.Y & 2)) * 2;

	attributeRegister1 |= ((((attributeLatch >> quadrant) & 1) ? 0xFF : 0) << 8);
	attributeRegister2 |= ((((attributeLatch >> quadrant) & 2) ? 0xFF : 0) << 8);
}


void PPU::pushPixel(u8 pixel) {

	u8 pallette_index = read_palette_data(pixel);

	u16 x = ((dot - 1) * 2);
	u16 y = (scanline * 2);

	u8 r = SYSTEM_PALETTE[3*pallette_index];
	u8 g = SYSTEM_PALETTE[3*pallette_index + 1];
	u8 b = SYSTEM_PALETTE[3*pallette_index + 2];

	screen::setColour(x, y, r, g, b);
}

void PPU::SpriteEvaluation() {


	if (dot == 0) { // Do nothing
		SecondaryOAMpointer = 0;
		OAMpointer = 0;
	}

	else if (dot <= 64) {
		if ((dot % 2) == 0) {
			secondaryOAM[SecondaryOAMpointer++] = 0xFF;
		}
	}

	else if (dot <= 256) {


		if (dot == 65) {
			SecondaryOAMpointer = 0;
			OAMpointer = 0;
		}

		if (mask.s || mask.b) {

			if (OAMpointer >= 256) {
				return;
			}

			if (SecondaryOAMpointer < 32) { // If we have not yet found 8 sprites, fill the secondary OAM table
				if ((dot % 2) == 1) {
					OAMpointerData = OAMdata[OAMpointer];
				}
				else {
					secondaryOAM[SecondaryOAMpointer] = OAMpointerData;

					if (((OAMpointer % 4) == 0) && !((OAMpointerData <= scanline) && (OAMpointerData + (ctrl.H ? 15 : 7) >= scanline))) { // If sprite not in range
						OAMpointer += 4;
					}
					else { // If sprite is in range

						if (!sprite_zero_hit && OAMpointer == 0) {
							sprite_zero_hit = true;
						}
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
					if (((OAMpointer % 4) == 0) && !((OAMpointerData <= scanline) && (OAMpointerData + (ctrl.H ? 15 : 7) >= scanline))) { // If sprite not in range
						OAMpointer += 4 + (((OAMpointer % 4) == 3) ? -3 : 1);
					}
					else { // Sprite is in range - so we have overflow
						spriteOverflow = true; // need to add register update here
						status.O = 1;
					}
				}
			}

		}

	}

	else if (dot <= 320) {

		if (dot == 257) {
			SecondaryOAMpointer = 0;
			SpriteRegistersPointer = 0;
		}

		if (mask.s || mask.b) {

			if ((dot % 8) == 1) {
				sprite_y = secondaryOAM[SecondaryOAMpointer++];
			}

			else if ((dot % 8) == 2) {
				sprite_tile = secondaryOAM[SecondaryOAMpointer++];
			}
			else if ((dot % 8) == 3) {
				spriteRegisters[++SpriteRegistersPointer] = secondaryOAM[SecondaryOAMpointer++] & 0xE3; // Attributes Data
				flipVertically = spriteRegisters[SpriteRegistersPointer] & 128;
			}
			else if ((dot % 8) == 4) {
				spriteRegisters[std::max(0, -1 + SpriteRegistersPointer++)] = secondaryOAM[SecondaryOAMpointer++]; // x_coordinate
			}

			else if ((dot % 8) == 6) { // Pattern low fetch

				u16 y_step = (sprite_y >= 242 ? 0 : scanline - sprite_y);

				if (ctrl.H) {

					u16 fine_y = (flipVertically ? 15 - y_step : y_step);
					u16 addr = ((sprite_tile & 1) << 12) + ((sprite_tile & ~1) << 4) + ((fine_y >= 8) ? 8 : 0) + fine_y;
 					spritePatternLow = read_PPU_data(addr);
				}
				else {
					u16 fine_y = (flipVertically ? 7 - y_step : y_step);
					u16 addr = (ctrl.S << 12) + (sprite_tile << 4) + fine_y;
					spritePatternLow = read_PPU_data(addr);
				}

				if (sprite_y >= 242) { // If sprite out of visible range, ignore pixel
					spritePatternLow = 0;
				}

				spriteRegisters[SpriteRegistersPointer++] = spritePatternLow;
			}

			else if ((dot % 8) == 0) { // Pattern high fetch

				u16 y_step = (sprite_y >= 242 ? 0 : scanline - sprite_y);

				if (ctrl.H) {
					u16 fine_y = (flipVertically ? 15 - scanline + sprite_y : scanline - sprite_y);
					u16 addr = ((sprite_tile & 1) << 12) + ((sprite_tile & ~1) << 4) + ((fine_y >= 8) ? 8 : 0) + fine_y + 8;
					spritePatternHigh = read_PPU_data(addr);
				}
				else {
					u16 fine_y = (flipVertically ? 7 - scanline + sprite_y : scanline - sprite_y);
					u16 addr = (ctrl.S << 12) + (sprite_tile << 4) + fine_y + 8;
					spritePatternHigh = read_PPU_data(addr);
				}

				if (sprite_y >= 242) { // If sprite out of visible range, ignore pixel
					spritePatternHigh = 0;
				}

				spriteRegisters[SpriteRegistersPointer++] = spritePatternHigh;
			}
		}
	}

}



u16 PPU::getMirrorAddress(u16 addr) {

	if (rom->getMirroring() == Mirroring::Horizantal) {

		if (addr < 0x2400) { // do nothing
		}

		else if (addr < 0x2800) {
			addr -= 0x400;
		}
		else if (addr < 0x2C00) {
			addr -= 0x400;
		}
		else if (addr < 0x3000) {
			addr -= 0x800;
		}

	}
	else if (rom->getMirroring() == Mirroring::Vertical) {

		if (addr >= 0x2800) {
			addr -= 0x800;
		}
	}

	else if (rom->getMirroring() == Mirroring::SingleScreen_Lower) {
		if (addr < 0x2400) { // do nothing
		}
		else if (addr < 0x2800) {
			addr -= 0x400;
		}
		else if (addr < 0x2C00) {
			addr -= 0x800;
		}
		else if (addr < 0x3000) {
			addr -= 0xC00;
		}
	}

	else if (rom->getMirroring() == Mirroring::SingleScreen_Upper) {
		if (addr < 0x2400) { // do nothing
		}
		else if (addr < 0x2800) {
			addr -= 0x400;
		}
		else if (addr < 0x2C00) {
			addr -= 0x800;
		}
		else if (addr < 0x3000) {
			addr -= 0xC00;
		}

		addr += 0x400;
	}

	return addr - 0x2000;
}

void PPU::incrementCoarseX() { // from nesdev wiki 

	if (register_v.X == 31) {
		register_v.X = 0;
		register_v.N ^= 1;  // switch horizontal nametable
	}
	else {
		register_v.X = (register_v.X + 1) & 0x1F;
	}
}

void PPU::incrementCoarseY() { // from nesdev wiki

	if (register_v.y < 7) {
		register_v.y = (register_v.y + 1) & 0x7;
	}
	else {
		register_v.y = 0;
		if (register_v.Y == 29) {
			register_v.Y = 0;
			register_v.N ^= 2;  // switch vertical nametable
		}
		else if (register_v.Y == 31) {
			register_v.Y = 0;
		}
		else {
			register_v.Y = (register_v.Y + 1) & 0x1F;;
		}
	}
}

u8 PPU::reverse(u8 n) {
	// Reverse the top and bottom nibble then swap them.
	return (lookup[n&0b1111] << 4) | lookup[n>>4];
}


void PPU::resetCycles() {
	cycles = 0;
}

bool PPU::NMI_triggered() {

	return NMI_enabled;

}

void PPU::set_NMI(bool status) {
	NMI_enabled = status;
}


bool PPU::immediate_NMI_triggered() {
	return immediate_NMI;
}


void PPU::set_immediate_NMI(bool status) {
	immediate_NMI = status;
}

u8 PPU::getCycles() {
	return cycles;
}

u16 PPU::getDot() {
	return dot;
}

u16 PPU::getScanline() {
	return scanline;
}

void PPU::print_pallette() { // Testing Function - side 0 or side 1

	for (int i { 0 }; i < 32; i++) {

		for (int x { 8 * i }; x < 8*i + 8; x++) {
			for (int y { 0 }; y < 8; y++) {

				u8 r = SYSTEM_PALETTE[3*palette_table[i]];
				u8 g = SYSTEM_PALETTE[3*palette_table[i] + 1];
				u8 b = SYSTEM_PALETTE[3*palette_table[i] + 2];
				screen::setColour(x, y, r, g, b);
			}
		}
	}

	screen::updateScreen();

}


void PPU::print_pattern(u8 side) { // Testing Function - side 0 or side 1

	for (int i { 0 }; i <= 0x0FFF; i++) {

		u16 row = i / 256;
		u16 column = (i % 256) / 16;

		u16 mini_row = i % 16;

		if (mini_row >= 8) { continue; }

		for (int j { 0 }; j <= 7; j++) {


			u8 x = ((column * 8) + (7 - j)) * 2;
			u8 y = ((row * 8)  + mini_row) * 2;

			u8 value = ((chr_rom[(side ? 0x1000 : 0) + i] >> j) & 1) + (((chr_rom[(side ? 0x1000 : 0) + i+8] >> j) & 1) << 1);

			switch (value) {
			case 0:
				screen::setColour(x, y, 255, 0, 0);
				break;
			case 1:
				screen::setColour(x, y, 0, 255, 0);
				break;
			case 2:
				screen::setColour(x, y, 0, 255, 255);
				break;
			case 3:
				screen::setColour(x, y, 0, 0, 255);
				break;
			}
		}
	}

	screen::updateScreen();

}


