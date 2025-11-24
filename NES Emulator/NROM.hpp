#ifndef NROM_H
#define NROM_H

#include "Mapper.hpp"

class NROM : public Mapper {

public:

	NROM(std::vector<u8>& prg, std::vector<u8>& chr, Mirroring mirroring) : Mapper(prg, chr, mirroring) {}
	~NROM() override = default;
	u8 read(u16 addr) override;
	void write(u16 addr, u8 data) override;

};

#endif