#include "ROM.hpp"
#include "typedef.hpp"
#include <stdexcept>
#include <string>
#include <string_view>
#include <fstream>
#include <iostream>

u8 ROM::read(u16 addr)
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

void ROM::write(u16 addr, u8 data)
{
	return;
};

ROM::ROM() // Default ROM for testing
	: prg_rom(32768), chr_rom(8192), mapper { 0 }, screen_mirroring { Mirroring::Horizantal }
{
};

ROM::ROM(std::vector<u8>& cartridge)
{
	if ((cartridge[0] != 0x4E) || (cartridge[1] != 0x45) || (cartridge[2] != 0x53) || (cartridge[3] != 0x1A)) {
		throw std::invalid_argument("not in iNES format");
	};

	mapper = (cartridge[7] & 0b11110000) | (cartridge[6] >> 4);

	if ((cartridge[6] & 1) == 0 ) {
		screen_mirroring = Mirroring::Horizantal;
	} 
	else if ((cartridge[6] & 1) == 1) {
		screen_mirroring = Mirroring::Vertical;
	}

	const u16 prg_size = cartridge[4] * 16384;
	const u16 chr_size = cartridge[5] * 8192;

	const u16 trainer = ((cartridge[6] & 0b100) != 0);

	const u16 prg_start = 16 + (trainer ? 512 : 0);
	const u16 chr_start = prg_start + prg_size;

	prg_rom = std::vector<u8>(cartridge.begin() + prg_start, cartridge.begin() + chr_start);
	chr_rom = std::vector<u8>(cartridge.begin() + chr_start, cartridge.end());
};

ROM::ROM(std::string file_path) {

	std::vector<char> header {};
	header.resize(16);

	std::ifstream file(file_path, std::ios_base::binary | std::ios_base::in);

	file.read(&header[0], 16);

	if (std::string { &header[0], &header[4] } != "NES\x1A") {
		throw std::invalid_argument("not in iNES format");
	}


	u8 mapper = (header[7] & 0b11110000) | (header[6] >> 4);

	if ((header[6] & 1) == 0) {
		screen_mirroring = Mirroring::Horizantal;
	}
	else if ((header[6] & 1) == 1) {
		screen_mirroring = Mirroring::Vertical;
	}

	const u8 prg_banks = header[4];
	const u8 chr_banks = header[5];

	const u8 trainer = ((header[6] & 0b100) != 0);

	prg_rom.resize(prg_banks * 16384);
	chr_rom.resize(chr_banks * 8192);

	if (prg_banks) {
		file.read(reinterpret_cast<char*>(&prg_rom[0]), prg_banks * 16384);
	}

	if (chr_banks) {
		file.read(reinterpret_cast<char*>(&chr_rom[0]), chr_banks * 8192);
	}

}
