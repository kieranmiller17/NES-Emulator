#include "Mapper.hpp"
#include <vector>


u8 Mapper::CHR_read(u16 addr) {
	return chr_rom[addr];
}

void Mapper::CHR_write(u16 addr, u8 data) {
	chr_rom[addr] = data;
}

bool Mapper::IRQ_Active() {
	return IRQ_request;
}

void Mapper::set_IRQ(bool status) {
	IRQ_request = status;
}

std::vector<u8>& Mapper::get_CHRrom() {
	return chr_rom;
}

Mirroring Mapper::getMirroring() {
	return screen_mirroring;
}